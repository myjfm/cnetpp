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
#include <cnetpp/tcp/event_center.h>
#include <cnetpp/tcp/event_poller.h>
#include <cnetpp/concurrency/thread.h>
#include <cnetpp/concurrency/task.h>
#include <cnetpp/base/log.h>

#include <thread>

namespace cnetpp {
namespace tcp {

namespace {
  const size_t kDefaultThreadNum = 5;
}

std::shared_ptr<EventCenter> EventCenter::New(const std::string& name,
    size_t thread_num) {
  if (thread_num <= 0) {
    thread_num = std::thread::hardware_concurrency();
  }
  if (thread_num <= 0) {
    thread_num = kDefaultThreadNum;
  }

  return std::shared_ptr<EventCenter>(new EventCenter(name, thread_num));
}

EventCenter::EventCenter(const std::string& name, size_t thread_num)
    : internal_event_poller_infos_(thread_num), name_(name) {
  for (size_t i = 0; i < thread_num; ++i) {
    internal_event_poller_infos_[i] =
        std::make_shared<InternalEventPollerInfo>();
    internal_event_poller_infos_[i]->event_poller_ = EventPoller::New(i);
    assert((internal_event_poller_infos_[i]->event_poller_).get());
  }
}

bool EventCenter::Launch() {
  for (size_t i = 0; i < internal_event_poller_infos_.size(); ++i) {
    if (!internal_event_poller_infos_[i]->event_poller_->Init(
          shared_from_this())) {
      Fatal("Failed to initialize EventPoller.");
    }
    Info("EventPoller %d initialized.", i);
    std::shared_ptr<concurrency::Task> task(
        new InternalEventTask(shared_from_this(),
                              internal_event_poller_infos_[i]->event_poller_));
    // Can not start the thread until put it into vector
    auto t = std::make_shared<concurrency::Thread>(task,
        name_ + "-poller-" + std::to_string(i));
    internal_event_poller_infos_[i]->event_poller_thread_ = t;
    t->Start();
  }
  return true;
}

void EventCenter::Shutdown() {
  for (auto& poller_info : internal_event_poller_infos_) {
    poller_info->event_poller_thread_->Stop();
  }
  internal_event_poller_infos_.clear();
}

void EventCenter::AddCommand(const Command& command, bool async) {
  int id = command.connection()->id() % internal_event_poller_infos_.size();

  auto& info = internal_event_poller_infos_[id];
  if (async) {
    {
      std::lock_guard<std::mutex> guard(info->pending_commands_mutex_);
      (info->pending_commands_).push_back(command);
    }

    info->event_poller_->Interrupt();
  } else {
    assert(command.connection()->ep_thread_id() == std::this_thread::get_id());
    ProcessPendingCommand(info, command);
  }
}

bool EventCenter::ProcessAllPendingCommands(size_t id) {
  if (id >= internal_event_poller_infos_.size()) {
    return false;
  }

  auto& info = internal_event_poller_infos_[id];
  if (info->event_poller_thread_->GetStatus() ==
      concurrency::Thread::Status::kStop) {
    return false;
  }

  std::vector<Command> pending_commands;
  {
    std::lock_guard<std::mutex> guard(info->pending_commands_mutex_);
    pending_commands.swap(info->pending_commands_);
  }

  for (auto& command : pending_commands) {
    ProcessPendingCommand(info, command);
  }
  return true;
}

void EventCenter::ProcessPendingCommand(InternalEventPollerInfoPtr info,
    const Command& command) {
  if (info->event_poller_->ProcessCommand(command)) {
    if (command.type() & static_cast<int>(Command::Type::kAddConn)) {
      info->connections_[command.connection()->socket().fd()] =
        command.connection();
      command.connection()->set_ep_thread_id();
      command.connection()->HandleReadableEvent(this);
    } else if (command.type() &
        static_cast<int>(Command::Type::kRemoveConnImmediately)) {
      (info->connections_).erase(command.connection()->socket().fd());
      command.connection()->HandleCloseConnection();
    } else if (command.type() &
        static_cast<int>(Command::Type::kRemoveConn)) {
      command.connection()->set_state(ConnectionBase::State::kClosing);
      command.connection()->HandleWriteableEvent(this);
    }
  }
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
  if (itr != connections.end()) {
    auto connection = itr->second;
    if (event.mask() & static_cast<int>(Event::Type::kClose)) {
      connection->MarkAsClosed(true);
    } else {
      if (event.mask() & static_cast<int>(Event::Type::kRead)) {
        connection->HandleReadableEvent(this);
      }
      if (event.mask() & static_cast<int>(Event::Type::kWrite)) {
        connection->HandleWriteableEvent(this);
      }
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

