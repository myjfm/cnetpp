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
#ifndef CNETPP_TCP_EVENT_H_
#define CNETPP_TCP_EVENT_H_

namespace cnetpp {
namespace tcp {

// When the EventPoller finds some socket is readable or writable, it will
// return an event
class Event {
 public:
  enum class Type {
    kDummy = 0x0,
    kRead = 0x01,
    kWrite = 0x02,
    kClose = 0x04,
  };

  explicit Event(int fd) : fd_(fd), mask_(static_cast<int>(Type::kDummy)) {
  }
  Event(int fd, int mask) : fd_(fd), mask_(mask) {
  }

  Event(Event&& e) {
    fd_ = e.fd_;
    mask_ = e.mask_;
  }
  Event& operator=(Event&& e) {
    fd_ = e.fd_;
    mask_ = e.mask_;
    return *this;
  }

  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;

  int fd() const {
    return fd_;
  }

  int mask() const {
    return mask_;
  }
  int& mutable_mask() {
    return mask_;
  }

 private:
  int fd_;
  int mask_;
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_EVENT_H_

