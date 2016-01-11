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
//   * Neither the name of Shuo Chen nor the names of other contributors may be
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
#include "tcp_connection.h"

#include <assert.h>

#include "command.h"
#include "event_center.h"
#include "../base/socket.h"

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
  event_center->AddCommand(command);
  return true;
}

bool TcpConnection::SendPacket(base::StringPiece data) {
  if (!send_buffer_.Write(data)) {
    return false;
  }
  return SendPacket();
}

// This method will be called when a socket fd becomes readable
void TcpConnection::HandleReadableEvent(EventCenter* event_center) {
  bool received = false;
  bool closed = false;

  if (!connected_) {
    int error = 0;
    socklen_t error_length = sizeof(error);
    if (!socket_.GetOption(SOL_SOCKET, SO_ERROR, &error, &error_length) ||
        (error && error != EINPROGRESS)) {
      closed = true;
    } else if (error == EINPROGRESS) {
      return;
    } else {
      if (connected_callback_) {
        // call callback user defined
        connected_callback_(
            std::static_pointer_cast<TcpConnection>(shared_from_this()));
      }
      connected_ = true;
    }
  }

  if (connected_) {
    // handle new arrival data
    struct iovec buffers[2];
    recv_buffer_.GetWritePositions(buffers, 2);
    while (true) {
      size_t received_length = 0;
      bool ret = socket_.Receive(buffers, 2, &received_length, true);
      status_ = cnetpp::concurrency::ThisThread::GetLastError();
      error_message_ = cnetpp::concurrency::ThisThread::GetLastErrorString();
      if (!ret && (status_ == EAGAIN || status_ == EWOULDBLOCK)) {
        break;
      } else if (!ret || received_length == 0) {
        closed = true;
        break;
      } else {
        // really received data
        recv_buffer_.CommitWrite(received_length);
        received = true;
        break;
      }
    }
  }

  if (received && received_callback_) {
    received_callback_(
        std::static_pointer_cast<TcpConnection>(shared_from_this()));
  }

  bool tmp = false;
  if (closed && closed_.compare_exchange_weak(tmp, closed)) {
    // remove this connection from event center
    Command command(static_cast<int>(Command::Type::kRemoveConn),
                    shared_from_this());
    event_center->AddCommand(command);
  }
}

void TcpConnection::HandleWriteableEvent(EventCenter* event_center) {
  if (send_buffer_.Size() <= 0) {
    if (connected_) {
      Command command(static_cast<int>(Command::Type::kReadable),
                      shared_from_this());
      event_center->AddCommand(command);
    }
    // do nothing
    return;
  }

  bool closed = false;
  if (!connected_) {
    int error = 0;
    socklen_t error_length = sizeof(error);
    if (!socket_.GetOption(SOL_SOCKET, SO_ERROR, &error, &error_length) ||
        (error && error != EINPROGRESS)) {
      closed = true;
    } else if (error == EINPROGRESS) {
      return;
    } else {
      connected_ = true;
      if (connected_callback_) {
        // call callback user defined
        connected_callback_(
            std::static_pointer_cast<TcpConnection>(shared_from_this()));
      }
    }
  }

  if (connected_) {
    size_t sent_length = 0;
    struct iovec buffers[2];
    send_buffer_.GetReadPositions(buffers, 2);
    bool ret = socket_.Send(buffers, 2, &sent_length, true);
    status_ = cnetpp::concurrency::ThisThread::GetLastError();
    error_message_ = cnetpp::concurrency::ThisThread::GetLastErrorString();
    if (!ret && status_ == EAGAIN) {
      return;
    } else if (!ret) {
      closed = true;
    } else {
      if (sent_length > 0) {
        int type = static_cast<int>(Command::Type::kReadable);
        if (sent_length != send_buffer_.Size()) {
          type |= static_cast<int>(Command::Type::kWriteable);
        }
        send_buffer_.CommitRead(sent_length);
        Command command(type, shared_from_this());
        event_center->AddCommand(command);
        if (sent_callback_) {
          sent_callback_(
              true,
              std::static_pointer_cast<TcpConnection>(shared_from_this()));
        }
      }
    }
  }

  bool tmp = false;
  if (closed && closed_.compare_exchange_weak(tmp, closed)) {
    // remove this connection from event center
    Command command(static_cast<int>(Command::Type::kRemoveConn),
                    shared_from_this());
    event_center->AddCommand(command);
  }
}

void TcpConnection::HandleCloseConnection() {
  assert(closed_.load(std::memory_order_relaxed));
  if (closed_callback_) {
    closed_callback_(
        std::static_pointer_cast<TcpConnection>(shared_from_this()));
  }
  socket_.Close();
}

void TcpConnection::MarkAsClosed() {
  bool tmp = false;
  if (closed_.compare_exchange_weak(tmp, true)) {
    // remove this connection from event center
    Command command(static_cast<int>(Command::Type::kRemoveConn),
                    shared_from_this());
    auto event_center = event_center_.lock();
    if (event_center.get()) {
      event_center->AddCommand(command);
    }
  }
}

}  // namespace tcp
}  // namespace cnetpp

