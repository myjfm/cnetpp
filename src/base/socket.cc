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
#include "socket.h"
#include <chrono>
#include <time.h>
#include <sys/time.h>

namespace cnetpp {
namespace base {

bool Socket::Create(int af, int type, int protocol) {
  Close();
  fd_ = ::socket(af, type, protocol);
  return IsValid();
}
 
void Socket::Attach(int fd) {
  if (fd != fd_) {
    bool res = Close();
    assert(res);
    fd_ = fd;
  }
}

bool Socket::Close() {
  if (IsValid()) {
    int fd = fd_;
    fd_ = -1;
    return ::close(fd) == 0;
  }
  return true;
}

bool Socket::SetCloexec(bool value) {
  int flags = fcntl(fd_, F_GETFD, 0);
  if (flags < 0) {
    return false;
  }

  if (value) {
    flags |= FD_CLOEXEC;
  } else {
    flags &= ~FD_CLOEXEC;
  }

  return fcntl(fd_, F_SETFD, flags) >= 0;
}

bool Socket::SetBlocking(bool value) {
  int flags = fcntl(fd_, F_GETFL, 0);
  if (flags < 0) {
    return false;
  }
  if (!value) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }
  return fcntl(fd_, F_SETFL, flags) == 0;
}

bool Socket::GetBlocking(bool* value) {
  int flags = fcntl(fd_, F_GETFL, 0);
  if (flags < 0) {
    return false;
  }
  *value = ((flags & O_NONBLOCK) == 0);
  return true;
}

bool Socket::Bind(const EndPoint& end_point) {
  sockaddr address;
  socklen_t address_len;
  if (!end_point.ToSockAddr(&address, &address_len)) {
    return false;
  }
  return ::bind(fd_, &address, address_len) == 0;
}


bool Socket::GetLocalEndPoint(EndPoint* end_point) const {
  struct sockaddr addr;
  socklen_t addr_length;
  if (getsockname(fd_, &addr, &addr_length) == 0) {
    end_point->FromSockAddr(addr, addr_length);
    return true;
  }
  return false;
}

bool Socket::GetPeerEndPoint(EndPoint* end_point) const {
  struct sockaddr addr;
  socklen_t addr_length;
  if (getpeername(fd_, &addr, &addr_length) == 0) {
    end_point->FromSockAddr(addr, addr_length);
    return true;
  }
  return false;
}

namespace {

inline int PollReadable(int fd, int64_t timeout_in_milliseconds = -1) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  return poll(&pfd, 1, timeout_in_milliseconds);
}

inline int PollReadable(int fd, struct timeval* timeout = nullptr) {
  int64_t timeout_in_milliseconds =
      timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
  return PollReadable(fd, timeout_in_milliseconds);
}

inline int PollWriteable(int fd, int64_t timeout_in_milliseconds = -1) {
  pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLOUT;
  return poll(&pfd, 1, timeout_in_milliseconds);
}

inline int PollWriteable(int fd, struct timeval* timeout = nullptr) {
  int timeout_in_milliseconds =
      timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
  return PollWriteable(fd, timeout_in_milliseconds);
}

}

bool Socket::WaitReadable(int64_t timeout_in_millisecond, bool restart) {
  while (true) {
    int n = PollReadable(fd_, timeout_in_millisecond);
    if (n != -1) {
      return n > 0;
    } else if (!IsInterruptedAndRestart(restart)) {
      break;
    }
  }
  return false;
}

bool Socket::WaitWriteable(int64_t timeout_in_milliseconds, bool restart) {
  while (true) {
    int n = PollWriteable(fd_, timeout_in_milliseconds);
    if (n != -1) {
      return n > 0;
    } else if (!IsInterruptedAndRestart(restart)) {
      break;
    }
  }
  return false;
}

// Following member methods are for ListenSocket
ListenSocket::ListenSocket(const EndPoint& end_point)
    : Socket(::socket(end_point.Family(), SOCK_STREAM, 0)) {
  if (!IsValid()) {
    assert(false);
    throw std::runtime_error("Create listen socket failed, end point is: " +
                             end_point.ToString());
  }
  
  SetReuseAddress(true);

  if (!Bind(end_point)) {
    throw std::runtime_error("Can't bind to " + end_point.ToString());
  }
}

bool ListenSocket::Accept(Socket* socket, bool auto_restart) {
  return Accept(socket, nullptr, auto_restart);
}

bool ListenSocket::Accept(Socket* socket,
                          EndPoint* end_point,
                          bool auto_restart) {
  assert(socket);
  struct sockaddr address;
  socklen_t address_length = sizeof(address);
  while (true) {
    int ret = accept(fd(), &address, &address_length);
    if (ret != -1) {
      socket->Attach(ret);
      if (end_point) {
        end_point->FromSockAddr(address, address_length);
      }
      return true;
    } else {
      if (!auto_restart || GetLastError() != EINTR) {
        break;
      }
    }
  }
  return false;
}

// Following member methods are for DataSocket
bool DataSocket::Connect(const EndPoint& end_point) {
  struct sockaddr address;
  socklen_t address_length = sizeof(address);
  end_point.ToSockAddr(&address, &address_length);
  if (connect(fd(), &address, address_length) != 0) {
    switch (errno) {
      case EINTR:
      case EWOULDBLOCK:
        return true;
      case EINPROGRESS: {
        bool blocking = true;
        if (GetBlocking(&blocking) && !blocking) {
          return true;
        }
      }
    }
    return false;
  }
  return true;
}

bool DataSocket::Send(const void* buffer,
                      size_t buffer_size,
                      size_t* sent_length,
                      int flags,
                      bool auto_restart) {
  while (true) {
    int n = send(fd(),
                 reinterpret_cast<const char*>(buffer),
                 buffer_size,
                 flags);
    if (n != -1) {
      *sent_length = n;
      return true;
    } else {
      if (!IsInterruptedAndRestart(auto_restart)) {
        *sent_length = 0;
        return false;
      }
    }
  }
}

bool DataSocket::Send(const struct iovec* buffer,
                      size_t count,
                      size_t* sent_length,
                      bool auto_restart) {
  while (true) {
    int n = writev(fd(), buffer, count);
    if (n != -1) {
      *sent_length = n;
      return true;
    } else {
      if (!IsInterruptedAndRestart(auto_restart)) {
        *sent_length = 0;
        return false;
      }
    }
  }
}

bool DataSocket::Receive(void* buffer,
                         size_t buffer_size,
                         size_t* received_size,
                         int flags,
                         bool auto_restart) {
  while (true) {
    int n = recv(fd(), reinterpret_cast<char*>(buffer), buffer_size, flags);
    if (n != -1) {
      *received_size = n;
      return true;
    } else if (!IsInterruptedAndRestart(auto_restart)) {
      *received_size = 0;
      return false;
    }
  }
}

bool DataSocket::Receive(struct iovec* buffer,
                         size_t count,
                         size_t* received_size,
                         bool auto_restart) {
  while (true) {
    int n = ::readv(fd(), buffer, count);
    if (n != -1) {  // connection closed
      *received_size = n;
      return true;
    } else if (!IsInterruptedAndRestart(auto_restart)) {  // error occured
      *received_size = 0;
      return false;
    }
  }
}

bool DataSocket::Receive(void* buffer,
                         size_t buffer_size,
                         size_t* received_size,
                         int64_t timeout_in_milliseconds,
                         int flags,
                         bool auto_restart) {
  if (WaitReadable(timeout_in_milliseconds, auto_restart)) {
    return Receive(buffer, buffer_size, received_size, flags, auto_restart);
  } else {
    return false;
  }
}

bool DataSocket::Receive(struct iovec* buffer,
                         size_t count,
                         size_t* received_size,
                         int64_t timeout_in_milliseconds,
                         bool auto_restart) {
  if (WaitReadable(timeout_in_milliseconds, auto_restart)) {
    return Receive(buffer, count, received_size, auto_restart);
  } else {
    return false;
  }
}

// Following member methods are for TcpSocket
bool TcpSocket::ReceiveAll(void *buffer,
                           size_t buffer_size,
                           size_t* received_size,
                           int flags,
                           bool auto_restart) {
  assert((flags & MSG_PEEK) == 0);
#ifdef MSG_WAITALL
  flags |= MSG_WAITALL;
#endif
  *received_size = 0;
  while (buffer_size > 0) {
    size_t n;
    if (Receive(buffer, buffer_size, &n, flags, auto_restart)) {
      if (n == 0) {
        SetLastError(ECONNRESET);
        return false;
      }
      buffer = reinterpret_cast<char *>(buffer) + n;
      buffer_size -= n;
      *received_size += n;
    } else {
      return false;
    }
  }

  return true;
}

bool TcpSocket::ReceiveAll(void *buffer,
                           size_t buffer_size,
                           int flags,
                           bool auto_restart) {
  size_t received_size;
  return ReceiveAll(buffer, buffer_size, &received_size, flags, auto_restart);
}

bool TcpSocket::ReceiveAll(void *buffer,
                           size_t buffer_size,
                           size_t* received_size,
                           int64_t timeout_in_milliseconds,
                           int flags,
                           bool auto_restart) {
  assert((flags & MSG_PEEK) == 0);
#ifdef MSG_WAITALL
  flags |= MSG_WAITALL;
#endif
  *received_size = 0;
  while (buffer_size > 0) {
    if (WaitReadable(timeout_in_milliseconds, auto_restart)) {
      size_t n;
      if (Receive(buffer, buffer_size, &n, flags, auto_restart)) {
        if (n == 0) {
          SetLastError(ECONNRESET);
          return false;
        }
        buffer = reinterpret_cast<char *>(buffer) + n;
        buffer_size -= n;
        *received_size += n;
      }
    } else {
      return false;
    }
  }

  return true;
}

bool TcpSocket::ReceiveAll(void *buffer,
                           size_t buffer_size,
                           int64_t timeout_in_milliseconds,
                           int flags,
                           bool auto_restart) {
  size_t received_size;
  return ReceiveAll(buffer,
                    buffer_size,
                    &received_size,
                    timeout_in_milliseconds,
                    flags,
                    auto_restart);
}

bool TcpSocket::ReceiveLine(void* buffer,
                            size_t buffer_size,
                            size_t* received_size,
                            size_t max_peek_size) {
  *received_size = 0;

  while (buffer_size > 0) {
    size_t peek_size = std::min(buffer_size, max_peek_size);
    size_t n;
    if (Receive(buffer, peek_size, &n, MSG_PEEK) && n > 0) {
      char* p = (char*)memchr(buffer, '\n', n);
      if (p) {
        bool result = ReceiveAll(buffer,
                                 p - reinterpret_cast<char*>(buffer) + 1,
                                 &n);
        *received_size += n;
        return result;
      } else {
        bool result = ReceiveAll(buffer, n, &n);
        *received_size += n;
        if (!result) {
          return false;
        }
        buffer = reinterpret_cast<char*>(buffer) + n;
        buffer_size -= n;
      }
    } else {
      return false;
    }
  }
  return false;
}

bool TcpSocket::ReceiveLine(std::string* str, size_t peek_size) {
  const size_t kMaxPeekSize = 1024;
  char buffer[kMaxPeekSize];
  peek_size = std::min(peek_size, kMaxPeekSize);

  str->clear();

  while (true) {
    size_t n;
    if (Receive(buffer, peek_size, &n, MSG_PEEK) && n > 0) {
      char* p = reinterpret_cast<char*>(memchr(buffer, '\n', n));
      if (p) {
        bool result = ReceiveAll(buffer, p - buffer + 1, &n);
        str->append(buffer, n);
        return result;
      } else {
        bool result = ReceiveAll(buffer, n, &n);
        str->append(buffer, n);
        if (!result) {
          return false;
        }
      }
    } else {
      return false;
    }
  }
  return false;
}

bool TcpSocket::SendAll(const void* buffer,
                        size_t buffer_size,
                        size_t* sent_size,
                        int flags,
                        bool auto_restart) {
  *sent_size = 0;
  while (buffer_size > 0) {
    size_t current_sent_size;
    if (Send(buffer, buffer_size, &current_sent_size, flags, auto_restart)) {
      buffer = reinterpret_cast<const char*>(buffer) + current_sent_size;
      buffer_size -= current_sent_size;
      *sent_size += current_sent_size;
    } else {
      return false;
    }
  }
  return true;
}

bool TcpSocket::SendAll(const void* buffer,
                           size_t buffer_size,
                           int flags,
                           bool auto_restart) {
  size_t sent_size;
  return SendAll(buffer, buffer_size, &sent_size, flags, auto_restart);
}

bool TcpSocket::SendAll(const void* buffer,
                        size_t buffer_size,
                        size_t* sent_size,
                        int64_t timeout_in_milliseconds,
                        int flags,
                        bool auto_restart) {
  *sent_size = 0;
  while (buffer_size > 0) {
    if (WaitWriteable(timeout_in_milliseconds, auto_restart)) {
      size_t current_sent_size;
      if (Send(buffer, buffer_size, &current_sent_size, flags, auto_restart)) {
        buffer = (const char*)buffer + current_sent_size;
        buffer_size -= current_sent_size;
        *sent_size += current_sent_size;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

// Following member methods are for UdpSocket
bool UdpSocket::SendTo(const void* buffer,
                       size_t buffer_size,
                       const EndPoint& end_point,
                       size_t* sent_size) {
  struct sockaddr address;
  socklen_t address_length = sizeof(address);
  end_point.ToSockAddr(&address, &address_length);
  int n = sendto(fd(),
                 reinterpret_cast<const char*>(buffer),
                 buffer_size,
                 0,
                 &address,
                 address_length);
  if (n >= 0) {
    *sent_size = n;
    return true;
  } else {
    *sent_size = 0;
    return false;
  }
}

bool UdpSocket::ReceiveFrom(void* buffer,
                            size_t buffer_size,
                            size_t* received_size,
                            EndPoint* end_point,
                            int flags) {
  struct sockaddr address;
  socklen_t address_length = sizeof(address);
  int n = recvfrom(fd(),
                   reinterpret_cast<char*>(buffer),
                   buffer_size,
                   flags,
                   &address,
                   &address_length);
  if (n >= 0) {
    *received_size = n;
    if (end_point) {
      end_point->FromSockAddr(address, address_length);
    }
    return true;
  } else {
    *received_size = 0;
  }
  return false;
}

}  // namespace base
}  // namespace cnetpp

