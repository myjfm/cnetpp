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
#ifndef CNETPP_BASE_SOCKET_H_
#define CNETPP_BASE_SOCKET_H_

#include <cnetpp/base/end_point.h>
#include <cnetpp/concurrency/this_thread.h>

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace cnetpp {
namespace base {

// socket base class
// please use StreamSocket, DatagramSocket or ListenSocket instead
class Socket {
 protected:
  Socket() : fd_(-1) {
  }
  explicit Socket(int fd) : fd_(fd) {
  }
  bool Create(int af, int type, int protocol = 0);
 
 public:
  virtual ~Socket() {
    Close();
  }
  int fd() const {
    return fd_;
  }
  bool IsValid() const {
    return fd_ > -1;
  }

  // Attach a fd to this object
  void Attach(int fd);

  // Detach fd from this object
  int Detach() {
    int fd = fd_;
    fd_ = -1;
    return fd;
  }

  bool Close();

  // Set the FD_CLOEXEC flag if value is nonzero,
  // or clear the flag if value is 0.
  // Return true on success, or false on error with errno set.
  bool SetCloexec(bool value = true);

  bool GetOption(int level, int name, void* value, socklen_t* length) const {
    return ::getsockopt(fd_,
                        level,
                        name,
                        reinterpret_cast<char*>(value),
                        length) == 0;
  }
  bool SetOption(int level, int name, const void* value, socklen_t length) {
    return ::setsockopt(fd_,
                        level,
                        name,
                        const_cast<char*>(static_cast<const char*>(value)),
                        length) == 0;
  }

  template <typename T>
  bool GetOption(int level, int name, T* value) const {
    socklen_t length = sizeof(value);
    return GetOption(level, name, value, &length);
  }

  template <typename T>
  bool SetOption(int level, int name, const T& value) {
    socklen_t length = sizeof(value);
    return SetOption(level, name, &value, length);
  }

  // Get socket option with difference type
  template <typename Type, typename InternalType>
  bool GetOption(int level, int name, Type* value) const {
    InternalType internal_value;
    bool result = GetOption(level, name, &internal_value);
    *value = static_cast<Type>(internal_value);
    return result;
  }

  // Set socket option with difference type
  template <typename Type, typename InternalType>
  bool SetOption(int level, int name, const Type& value) {
    return SetOption(level, name, static_cast<InternalType>(value));
  }

  bool GetOption(int level, int name, bool* value) const {
    int int_value;
    bool result = GetOption(level, name, &int_value);
    *value = (int_value != 0);
    return result;
  }
  bool SetOption(int level, int name, const bool& value) {
    return SetOption(level, name, static_cast<int>(value));
  }

  bool GetError(int* error) {
    return GetOption(SOL_SOCKET, SO_ERROR, error);
  }
  bool GetType(int* type) const {
    return GetOption(SOL_SOCKET, SO_TYPE, type);
  }

  bool GetSendBufferSize(size_t* size) const{
    return GetOption<size_t, int>(SOL_SOCKET, SO_SNDBUF, size);
  }
  bool SetSendBufferSize(size_t size) {
    return SetOption<size_t, int>(SOL_SOCKET, SO_SNDBUF, size);
  }
  bool GetReceiveBufferSize(size_t* size) const {
    return GetOption<size_t, int>(SOL_SOCKET, SO_RCVBUF, size);
  }
  bool SetReceiveBufferSize(size_t size) {
    return SetOption<size_t, int>(SOL_SOCKET, SO_RCVBUF, size);
  }

  bool SetTcpUserTimeout(int milliseconds = 60000) {
#if __linux__
    return SetOption<int, unsigned int>(IPPROTO_TCP,
                                        TCP_USER_TIMEOUT,
                                        milliseconds);
#endif
    return true;
  }

  bool SetSendTimeout(const timeval& tv) {
    return SetOption(SOL_SOCKET, SO_SNDTIMEO, tv);
  }
  bool SetSendTimeout(int milliseconds) {
    timeval tv = {milliseconds / 1000, milliseconds % 1000};
    return SetSendTimeout(tv);
  }
  bool SetReceiveTimeout(const timeval& tv) {
    return SetOption(SOL_SOCKET, SO_RCVTIMEO, tv);
  }
  bool SetReceiveTimeout(int milliseconds) {
    timeval tv = {milliseconds / 1000, milliseconds % 1000};
    return SetReceiveTimeout(tv);
  }
  bool Ioctl(int cmd, int* value) {
    return ::ioctl(fd_, cmd, reinterpret_cast<void*>(value)) == 0;
  }

  bool SetBlocking(bool value = true);
  bool GetBlocking(bool* value);

  bool Bind(const EndPoint& end_point);

  bool GetLocalEndPoint(EndPoint* end_point) const;

  bool GetPeerEndPoint(EndPoint* end_point) const;

  bool GetReuseAddress(bool* value) {
    return GetOption(SOL_SOCKET, SO_REUSEADDR, value);
  }
  bool SetReuseAddress(bool value = true) {
    return SetOption(SOL_SOCKET, SO_REUSEADDR, value);
  }

  bool SetLinger(bool onoff = true, int timeout = 0) {
    struct linger l;
    l.l_onoff = onoff;
    l.l_linger = (u_short) timeout;
    return SetOption(SOL_SOCKET, SO_LINGER, l);
  }

  bool GetKeepAlive(bool* onoff) {
    return GetOption(SOL_SOCKET, SO_KEEPALIVE, onoff);
  }
  bool SetKeepAlive(bool onoff = true) {
    return SetOption(SOL_SOCKET, SO_KEEPALIVE, onoff);
  }

  bool SetTcpKeepAliveOption(int idle, int interval, int count) {
#if __linux__
    return SetOption(SOL_SOCKET, SO_KEEPALIVE, 1) &&
           SetOption(IPPROTO_TCP, TCP_KEEPIDLE, idle) &&
           SetOption(IPPROTO_TCP, TCP_KEEPINTVL, interval) &&
           SetOption(IPPROTO_TCP, TCP_KEEPCNT, count);
#elif __APPLE__
    return SetOption(SOL_SOCKET, SO_KEEPALIVE, 1);
#endif
  }

  bool SetTcpNoDelay(bool onoff = true) {
    return SetOption(IPPROTO_TCP, TCP_NODELAY, onoff);
  }
  bool GetTcpNoDelay(bool* onoff) {
    return GetOption(IPPROTO_TCP, TCP_NODELAY, onoff);
  }

  // @param timeout   nullptr means no timeout
  bool WaitReadable(struct timeval* timeout = nullptr, bool restart = true) {
    int64_t timeout_in_milliseconds =
        timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
    return WaitReadable(timeout_in_milliseconds, restart);
  }
  // @param timeout_in_milliseconds   -1 means no timeout
  bool WaitReadable(int64_t timeout_in_milliseconds, bool restart = true);

  // @param timeout   nullptr means no timeout
  bool WaitWriteable(struct timeval* timeout = nullptr, bool restart = true) {
    int64_t timeout_in_milliseconds =
        timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
    return WaitWriteable(timeout_in_milliseconds, restart);
  }
  // @param timeout_in_milliseconds   -1 means no timeout
  bool WaitWriteable(int64_t timeout_in_milliseconds, bool restart = true);

  bool IsReadable() {
    return WaitReadable(static_cast<int64_t>(0));
  }
  bool IsWriteable() {
    return WaitWriteable(static_cast<int64_t>(0));
  }

 public:
  static int GetLastError() {
    return cnetpp::concurrency::ThisThread::GetLastError();
  }
  static std::string GetLastErrorString() {
    return cnetpp::concurrency::ThisThread::GetLastErrorString();
  }

 protected:
  static void SetLastError(int error) {
    cnetpp::concurrency::ThisThread::SetLastError(error);
  }
  static void VerifyFd(int fd) {
    assert(fd != -1);
  }
  static bool IsInterruptedAndRestart(bool restart) {
    return (restart && (GetLastError() == EINTR));
  }

 private:
  Socket(const Socket&);
  Socket& operator=(const Socket&);

  int fd_;
};

// Listen streaming connections from client
class ListenSocket : public Socket {
 public:
  ListenSocket() {}
  ListenSocket(const EndPoint& end_point);
  bool Create(bool ipv6 = false) {
    return Socket::Create(ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
  }
  bool Listen(int backlog = SOMAXCONN) {
    return listen(fd(), backlog) == 0;
  }

  bool Accept(Socket* socket, bool auto_restart = true);
  bool Accept(Socket* socket, EndPoint* end_point, bool auto_restart = true);
};

// Abstract data transfer socket
class DataSocket : public Socket {
 protected:
  DataSocket() {}
 
 public:
  // -1 means error occured
  // 0 means we are connecting
  // 1 means connected
  int Connect(const EndPoint& end_point);

  // Send data
  bool Send(const void* buffer,
            size_t buffer_size,
            size_t* sent_length,
            int flags = 0,
            bool auto_restart = true);

  bool Send(const struct iovec* buffer,
            size_t count,
            size_t* sent_length,
            bool auto_restart = true);

  // @return Whether received any data or connect close by peer.
  // @note If connection is closed by peer, return true and received_size
  //       is set to 0.
  bool Receive(void* buffer,
               size_t buffer_size,
               size_t* received_size,
               int flags = 0,
               bool auto_restart = true);

  bool Receive(struct iovec* buffer,
               size_t count,
               size_t* received_size,
               bool auto_restart = true);

  // Receive with timeout
  // @return false if error or timeout, check Socket::GetLastError() for details
  bool Receive(void* buffer,
               size_t buffer_size,
               size_t* received_size,
               timeval* timeout,
               int flags = 0,
               bool auto_restart = true) {
    int64_t timeout_in_milliseconds =
        timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
    return Receive(buffer,
                   buffer_size,
                   received_size,
                   timeout_in_milliseconds,
                   flags,
                   auto_restart);
  }

  bool Receive(struct iovec* buffer,
               size_t count,
               size_t* received_size,
               timeval* timeout,
               bool auto_restart = true) {
    int64_t timeout_in_milliseconds =
        timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
    return Receive(buffer,
                   count,
                   received_size,
                   timeout_in_milliseconds,
                   auto_restart);
  }

  bool Receive(void* buffer,
               size_t buffer_size,
               size_t* received_size,
               int64_t timeout_in_milliseconds,
               int flags = 0,
               bool auto_restart = true);

  bool Receive(struct iovec* buffer,
               size_t count,
               size_t* received_size,
               int64_t timeout_in_milliseconds,
               bool auto_restart = true);
};

/// Tcp socket
class TcpSocket : public DataSocket {
 public:
  TcpSocket() {}

  // Create a stream socket
  bool Create(bool ipv6 = false) {
    return Socket::Create((ipv6 ? AF_INET6 : AF_INET), SOCK_STREAM, 0);
  }

  // Shutdown connection
  bool Shutdown() {
    return shutdown(fd(), SHUT_RDWR) == 0;
  }

  // Shutdown connection sending
  bool ShutdownSend() {
    return shutdown(fd(), SHUT_WR) == 0;
  }

  // Shutdown connection receiving
  bool ShutdownReceive() {
    return shutdown(fd(), SHUT_RD) == 0;
  }

  // @brief Receive data of all expected length
  // @return Whether received all expacted data
  // @note If return false, data may also received and received will be
  //       greater than 0
  bool ReceiveAll(void *buffer,
                  size_t buffer_size,
                  size_t* received_size,
                  int flags = 0,
                  bool auto_restart = true);

  // @brief Same as upper, expect without the out param 'received_size'
  bool ReceiveAll(void *buffer,
                  size_t buffer_size,
                  int flags = 0,
                  bool auto_restart = true);

  // @brief Receive all length, with timeout and out param received_size
  // @return Whether received all data
  bool ReceiveAll(void *buffer,
                  size_t buffer_size,
                  size_t* received_size,
                  timeval* timeout,
                  int flags = 0,
                  bool auto_restart = true) {
    int64_t timeout_in_milliseconds =
        timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
    return ReceiveAll(buffer,
                      buffer_size,
                      received_size,
                      timeout_in_milliseconds,
                      flags,
                      auto_restart);
  }

  bool ReceiveAll(void *buffer,
                  size_t buffer_size,
                  size_t* received_size,
                  int64_t timeout_in_milliseconds,
                  int flags = 0,
                  bool auto_restart = true);

  // @brief Receive all length, with timeout
  bool ReceiveAll(void *buffer,
                  size_t buffer_size,
                  timeval* timeout,
                  int flags = 0,
                  bool auto_restart = true) {
    int64_t timeout_in_milliseconds =
        timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
    return ReceiveAll(buffer,
                      buffer_size,
                      timeout_in_milliseconds,
                      flags,
                      auto_restart);
  }

  bool ReceiveAll(void *buffer,
                  size_t buffer_size,
                  int64_t timeout_in_milliseconds,
                  int flags = 0,
                  bool auto_restart = true);

  // Receive a line to buffer, include terminal '\n'
  // @return Whether received a complete line
  bool ReceiveLine(void* buffer,
                   size_t buffer_size,
                   size_t* received_size,
                   size_t max_peek_size = 80);

  // Receive a line to string, include terminal '\n'
  // @return Whether received a complete line
  bool ReceiveLine(std::string* str, size_t peek_size = 80);

  // Send all data of buffer to socket
  // @return Whether all data sent
  bool SendAll(const void* buffer,
               size_t buffer_size,
               size_t* sent_size,
               int flags = 0,
               bool auto_restart = true);

  // @brief Send all buffer to socket
  // @return true if all data sent, flase for any other case
  // @note If false returned, partial data may alse be sent
  bool SendAll(const void* buffer,
               size_t buffer_size,
               int flags = 0,
               bool auto_restart = true);

  // @brief Send all buffer to socket with timeout
  // @return true if all data sent, flase for any other case
  // @note If false returned, partial data may alse be sent
  bool SendAll(const void* buffer,
               size_t buffer_size,
               size_t* sent_size,
               timeval* timeout,
               int flags = 0,
               bool auto_restart = true) {
    int64_t timeout_in_milliseconds =
        timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : -1;
    return SendAll(buffer,
                   buffer_size,
                   sent_size,
                   timeout_in_milliseconds,
                   flags,
                   auto_restart);
  }

  bool SendAll(const void* buffer,
               size_t buffer_size,
               size_t* sent_size,
               int64_t timeout_in_milliseconds,
               int flags = 0,
               bool auto_restart = true);
};

/// Represent a Udp socket
class UdpSocket : public DataSocket {
 public:
  // Construct an empty object
  UdpSocket() {}

  // Create the system socket
  bool Create(bool ipv6 = false) {
    return Socket::Create((ipv6 ? AF_INET6 : AF_INET), SOCK_DGRAM, 0);
  }

  // Send data with specified address
  bool SendTo(const void* buffer,
              size_t buffer_size,
              const EndPoint& end_point,
              size_t* sent_size);

  /// Receive data and obtain remote address
  bool ReceiveFrom(void* buffer,
                   size_t buffer_size,
                   size_t* received_size,
                   EndPoint* end_point,
                   int flags = 0);
};

}  // namespace base
}  // namespace cnetpp

#endif  // CNETPP_BASE_SOCKET_H_

