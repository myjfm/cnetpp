// Copyright (c) 2015, myjfm(mwxjmmyjfm@gmail.com).  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//   * Neither the name of myjfm nor the names of other contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
#include "event_center.h"

#include <thread>

#include "event_poller.h"
#include "../concurrency/thread_manager.h"
#include "../concurrency/task.h"

namespace cnetpp {
namespace tcp {

namespace {
  const size_t kDefaultThreadNum = 5;
}

std::shared_ptr<EventCenter> EventCenter::New(size_t thread_num) {
  if (thread_num <= 0) {
    thread_num = std::thread::hardware_concurrency();
  }
  if (thread_num <= 0) {
    thread_num = kDefaultThreadNum;
  }

  return std::shared_ptr<EventCenter>(new EventCenter(thread_num));
}

EventCenter::EventCenter(size_t thread_num)
    : internal_event_poller_infos_(thread_num) {
  for (size_t i = 0; i < thread_num; ++i) {
    internal_event_poller_infos_[i] =
        std::make_shared<InternalEventPollerInfo>();
    internal_event_poller_infos_[i]->event_poller_ = EventPoller::New(i);
    assert((internal_event_poller_infos_[i]->event_poller_).get());
  }
}

bool EventCenter::Launch() {
  for (size_t i = 0; i < internal_event_poller_infos_.size(); ++i) {
    if (!internal_event_poller_infos_[i]->event_poller_->Init(shared_from_this())) {
      continue;
    }
    std::shared_ptr<concurrency::Task> task(
        new InternalEventTask(shared_from_this(),
                              internal_event_poller_infos_[i]->event_poller_));
    internal_event_poller_infos_[i]->event_poller_thread_ =
        concurrency::ThreadManager::Instance()->CreateThread(task);
  }
  return true;
}

void EventCenter::Shutdown() {
  for (auto& poller_info : internal_event_poller_infos_) {
    poller_info->event_poller_->Interrupt();
    poller_info->event_poller_thread_->Stop();
  }
  internal_event_poller_infos_.clear();
}

void EventCenter::AddCommand(const Command& command, int id) {
  if (id < 0) {
    id = command.connection()->id() % internal_event_poller_infos_.size();
  }

  auto& info = internal_event_poller_infos_[id];
  {
    std::unique_lock<std::mutex> guard(info->pending_commands_mutex_);
    (info->pending_commands_).push_back(command);
  }

  info->event_poller_->Interrupt();
}

bool EventCenter::ProcessAllPendingCommands(size_t id) {
  if (id >= internal_event_poller_infos_.size()) {
    return false;
  }

  auto& info = internal_event_poller_infos_[id];
  std::unique_lock<std::mutex> guard(info->pending_commands_mutex_);
  std::vector<Command> pending_commands;
  pending_commands.swap(info->pending_commands_);
  guard.unlock();

  for (auto& command : pending_commands) {
    if (info->event_poller_->ProcessCommand(command)) {
      if (command.type() & static_cast<int>(Command::Type::kAddConn)) {
        info->connections_[command.connection()->socket().fd()] =
            command.connection();
        command.connection()->HandleReadableEvent(this);
      }
      if (command.type() & static_cast<int>(Command::Type::kRemoveConn)) {
        (info->connections_).erase(command.connection()->socket().fd());
        command.connection()->HandleCloseConnection();
      }
    }
  }
  return true;
}

bool EventCenter::ProcessEvent(const Event& event, size_t id) {
  if (id >= internal_event_poller_infos_.size()) {
    return false;
  }

  int fd = event.fd();
  if (fd < 0) {
    return false;
  }

  auto& connections = internal_event_poller_infos_[id]->connections_;
  auto itr = connections.find(event.fd());
  if (event.mask() & static_cast<int>(Event::Type::kClose)) {
    if (itr != connections.end()) {
      auto connection = itr->second;
      connections.erase(itr);
      connection->HandleCloseConnection();
    } /* else { // this connection has been closed. } */
  } else {
    if (event.mask() & static_cast<int>(Event::Type::kRead)) {
      if (itr != connections.end()) {
        itr->second->HandleReadableEvent(this);
      } /* else { // this connection has been closed. } */
    }
    if (event.mask() & static_cast<int>(Event::Type::kWrite)) {
      if (itr != connections.end()) {
        itr->second->HandleWriteableEvent(this);
      } /* else { // this connection has been closed. } */
    }
  }

  return true;
}

EventCenter::InternalEventTask::InternalEventTask(
    std::weak_ptr<EventCenter> event_center,
    std::weak_ptr<EventPoller> event_poller)
    : event_center_(event_center),
      event_poller_(event_poller) {
}

EventCenter::InternalEventTask::~InternalEventTask() {
}

bool EventCenter::InternalEventTask::operator()(void* arg) {
  (void)arg;
  auto event_poller = event_poller_.lock();
  auto event_center = event_center_.lock();
  if (!event_poller || !event_center) {
    return false;
  }

  while (!IsStopped()) {
    if (!event_poller->Poll()) {
      event_poller->Shutdown();
      return false;
    }
  }
  event_poller->Shutdown();
  return true;
}

void EventCenter::InternalEventTask::Stop() {
  stop_.store(true, std::memory_order_release);
  auto event_poller = event_poller_.lock();
  if (event_poller) {
    event_poller->Interrupt();
  }
}

}  // namespace tcp
}  // namespace cnetpp

