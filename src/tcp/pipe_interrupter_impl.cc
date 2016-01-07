#include <tcp/pipe_interrupter_impl.h>

#include <unistd.h>
#include <fcntl.h>

namespace cnetpp {
namespace tcp {

PipeInterrupterImpl::~PipeInterrupterImpl() {
  if (read_fd_ != -1) {
    ::close(read_fd_);
  }
  if (write_fd_ != -1) {
    ::close(write_fd_);
  }
}

bool PipeInterrupterImpl::Create() {
  int pipe_fds[2] { -1, -1};
  if (::pipe(pipe_fds) < 0) {
    return false;
  }

  read_fd_ = pipe_fds[0];
  ::fcntl(read_fd_, F_SETFL, O_NONBLOCK);
  ::fcntl(read_fd_, F_SETFD, FD_CLOEXEC);

  write_fd_ = pipe_fds[1];
  ::fcntl(write_fd_, F_SETFL, O_NONBLOCK);
  ::fcntl(write_fd_, F_SETFD, FD_CLOEXEC);
  return true;
}

// interrupt the epoll_wait call.
bool PipeInterrupterImpl::Interrupt() {
  char byte = 0;
  if (write_fd_ < 0 || ::write(write_fd_, &byte, 1) < 0) {
    return false;
  }
  return true;
}

// Reset the epoll interrupt.
// Returns true if the epoll_wait call was interrupted.
bool PipeInterrupterImpl::Reset() {
  char data[64];
  int bytes_read = ::read(read_fd_, data, sizeof(data));
  bool was_interrupted = (bytes_read > 0);
  while (bytes_read == sizeof(data)) {
    bytes_read = ::read(read_fd_, data, sizeof(data));
  }
  return was_interrupted;
}

}  // namespace tcp
}  // namespace cnetpp

