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
#include <cnetpp/tcp/tcp_connection.h>
#include <cnetpp/tcp/command.h>
#include <cnetpp/tcp/ring_buffer.h>
#include <cnetpp/tcp/event_center.h>
#include <cnetpp/base/socket.h>
#include <cnetpp/base/log.h>
#include <assert.h>

#include <memory>

namespace cnetpp {
namespace tcp {

bool TcpConnection::SendPacket() {
  Command command(static_cast<int>(Command::Type::kReadable) |
                  static_cast<int>(Command::Type::kWriteable),
                  shared_from_this());
  std::shared_ptr<EventCenter> event_center = event_center_.lock();
  if (!event_center.get()) {
    return false;
  }
  CnetppDebug("[Socket 0X%08x] [TcpConnection 0X%08x] "
              "send packet command [type %s]",
              socket_.fd(), this->id(), command.TypeString().c_str());
  event_center->AddCommand(command,
      ep_thread_id_ != std::this_thread::get_id());
  return true;
}

bool TcpConnection::SendPacket(base::StringPiece data) {
  auto send_buffer = std::make_unique<RingBuffer>(data.size());
  bool r = send_buffer->Write(data);
  (void) r;
  assert(r);
  return SendPacket(std::move(send_buffer));
}

bool TcpConnection::SendPacket(std::unique_ptr<RingBuffer>&& data) {
  {
    concurrency::SpinLock::ScopeGuard guard(send_lock_);
    send_buffers_.emplace_back(std::move(data));
  }
  return SendPacket();
}

// This method will be called when a socket fd becomes readable
void TcpConnection::HandleReadableEvent(EventCenter* event_center) {
  CnetppDebug("[Socket 0X%08x] [TcpConnection 0X%08x] "
              "receive readable event", socket_.fd(), this->id());
  bool closed = false;

  if (state_ == State::kConnecting) {
    int error = 0;
    socklen_t error_length = sizeof(error);
    if (!socket_.GetOption(SOL_SOCKET, SO_ERROR, &error, &error_length) ||
        (error && error != EINPROGRESS)) {
      closed = true;
    } else if (error == EINPROGRESS) {
      return;
    } else {
      CnetppDebug("[Socket 0X%08x] [TcpConnection 0X%08x] "
                  "change state from kConnecting to kConnected",
                  socket_.fd(), this->id());
      state_ = State::kConnected;
      if (connected_callback_) {
        // call callback user defined
        connected_callback_(
            std::static_pointer_cast<TcpConnection>(shared_from_this()));
      }
    }
  }

  if (state_ == State::kConnected) {
    // handle new arrival data
    while (true) {
      if (recv_buffer_.Capacity() - recv_buffer_.Size() < 512) {
        recv_buffer_.Resize(2 * recv_buffer_.Capacity());
      }
      struct iovec buffers[2];
      recv_buffer_.GetWritePositions(buffers, 2);
      size_t received_length = 0;
      bool ret = socket_.Receive(buffers, 2, &received_length, true);
      status_ = cnetpp::concurrency::ThisThread::GetLastError();
      //error_message_ = cnetpp::concurrency::ThisThread::GetLastErrorString();
      if (!ret && (status_ == EAGAIN || status_ == EWOULDBLOCK)) {
        break;
      } else if (!ret || received_length == 0) {
        closed = true;
        break;
      } else {
        // really received data
        recv_buffer_.CommitWrite(received_length);
        if (received_callback_) {
          if (!received_callback_(
              std::static_pointer_cast<TcpConnection>(shared_from_this()))) {
            closed = true;
            break;
          }
        }
      }
    }
  }

  if (closed && state_ != State::kClosed) {
    // remove this connection from event center
    Command command(static_cast<int>(Command::Type::kRemoveConnImmediately),
                    shared_from_this());
    event_center->AddCommand(command, false/* only ep thread could be here */);
  }
}

void TcpConnection::HandleWriteableEvent(EventCenter* event_center) {
  CnetppDebug("[Socket 0X%08x] [TcpConnection 0X%08x] "
              "receive writeable event...", socket_.fd(), this->id());

  bool closed = false;

  if (state_ == State::kConnecting) {
    int error = 0;
    socklen_t error_length = sizeof(error);
    if (!socket_.GetOption(SOL_SOCKET, SO_ERROR, &error, &error_length) ||
        (error && error != EINPROGRESS)) {
      std::string error_string = concurrency::ThisThread::GetLastErrorString();
      CnetppInfo("[Socket 0X%08x] [TcpConnection 0X%08x] "
                 "GetOptions error %d %s", socket_.fd(), this->id(),
                 concurrency::ThisThread::GetLastError(), error_string.c_str());
      closed = true;
    } else if (error == EINPROGRESS) {
      return;
    } else {
      cnetpp::base::EndPoint ep;
      this->socket().GetLocalEndPoint(&ep);
      CnetppDebug("[Socket 0X%08x] [TcpConnection 0X%08x] change state "
                  "from kConnecting to kConnected, [LocalEndPoint %s]",
                  socket_.fd(), this->id(), ep.ToString().c_str());
      state_ = State::kConnected;
      if (connected_callback_) {
        // call callback user defined
        connected_callback_(
            std::static_pointer_cast<TcpConnection>(shared_from_this()));
      }
    }
  }

  if (!closed) {
    send_lock_.Lock();
    if (send_buffers_.empty()) {
      send_lock_.Unlock();
      if (state_ == State::kConnected) {
        Command command(static_cast<int>(Command::Type::kReadable),
            shared_from_this());
        event_center->AddCommand(command, false);
      } else if (state_ == State::kClosing) {
        Command command(static_cast<int>(Command::Type::kRemoveConnImmediately),
            shared_from_this());
        event_center->AddCommand(command, false);
      }
      // do nothing
      return;
    }
    send_lock_.Unlock();
  }

  if (state_ == State::kConnected || state_ == State::kClosing) {
    while (true) {
      send_lock_.Lock();
      auto& send_buffer = send_buffers_.front();
      send_lock_.Unlock();
      size_t sent_length = 0;
      struct iovec buffers[2];
      send_buffer->GetReadPositions(buffers, 2);
      bool ret = socket_.Send(buffers, 2, &sent_length, true);
      status_ = cnetpp::concurrency::ThisThread::GetLastError();
      //error_message_ = cnetpp::concurrency::ThisThread::GetLastErrorString();
      if (!ret && status_ == EAGAIN) {
        return;
      } else if (!ret) {
        closed = true;
        break;
      } else {
        if (sent_length > 0) {
          if (sent_length != send_buffer->Size()) {
            send_buffer->CommitRead(sent_length);
            int type = static_cast<int>(Command::Type::kReadable) |
              static_cast<int>(Command::Type::kWriteable);
            event_center->AddCommand(Command(type, shared_from_this()), false);
            return;
          } else {
            bool all_sent = false;
            send_lock_.Lock();
            send_buffers_.pop_front();
            if (send_buffers_.empty()) {
              all_sent = true;
            }
            send_lock_.Unlock();
            if (all_sent && state_ != State::kClosing) {
              int type = static_cast<int>(Command::Type::kReadable);
              event_center->AddCommand(Command(type, shared_from_this()),
                  false);
            }
            if (sent_callback_) {
              sent_callback_(true,
                  std::static_pointer_cast<TcpConnection>(shared_from_this()));
            }
            if (all_sent) {
              if (state_ == State::kClosing) {
                closed = true;
              }
              break;
            }
          }
        }
      }
    }
  }

  if (closed && state_ != State::kClosed) {
    Command command(static_cast<int>(Command::Type::kRemoveConnImmediately),
                    shared_from_this());
    event_center->AddCommand(command, false);
  }
}

void TcpConnection::HandleCloseConnection() {
  CnetppDebug("[Socket 0X%08x] [TcpConnection 0X%08x] "
              "receive close event", socket_.fd(), this->id());

  if (state_ == State::kClosed) {
    return;
  }
  state_ = State::kClosed;
  if (closed_callback_) {
    closed_callback_(
        std::static_pointer_cast<TcpConnection>(shared_from_this()));
  }
  socket_.Close();
}

void TcpConnection::MarkAsClosed(bool immediately) {
  CnetppDebug("[Socket 0X%08x] [TcpConnection 0X%08x] "
              "receive mark closed event, owing to Direct call, "
              "AsyncClose or Poll error", socket_.fd(), this->id());
  int type = 0;
  if (immediately) {
    type = static_cast<int>(Command::Type::kRemoveConnImmediately);
  } else {
    type = static_cast<int>(Command::Type::kRemoveConn);
  }
  Command command(type, shared_from_this());
  auto event_center = event_center_.lock();
  if (event_center.get()) {
    event_center->AddCommand(command,
        ep_thread_id_ != std::this_thread::get_id());
  }
}

}  // namespace tcp
}  // namespace cnetpp

