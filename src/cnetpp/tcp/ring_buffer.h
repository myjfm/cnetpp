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
#ifndef CNETPP_TCP_RING_BUFFER_H_
#define CNETPP_TCP_RING_BUFFER_H_

#include <cnetpp/base/string_piece.h>

#include <sys/uio.h>
#include <string.h>

#include <string>
#include <vector>

namespace cnetpp {
namespace tcp {

// buffer class designed for TCP connection
// NOTE: You should not use this class across multi-threads unless you know how
// this class is implemented
class RingBuffer {
 public:
  explicit RingBuffer(size_t buffer_size)
      : buffer_(new char[buffer_size]),
        begin_(0),
        end_(0),
        size_(0),
        capacity_(buffer_size) {
    assert(buffer_);
  }
  ~RingBuffer() {
    if (buffer_) {
      delete [] buffer_;
    }
  }

  size_t Capacity() {
    return capacity_;
  }

  size_t Size() {
    return size_;
  }

  // if new_size is less than size_, resize will fail
  bool Resize(size_t new_size);

  size_t Length() {
    return size_;
  }

  bool Full() {
    return size_ == capacity_;
  }

  bool Empty() {
    return size_ == 0;
  }

  // NOTE: After called GetWritePositions(),
  // if you indeed write some data into this buffer,
  // you must call CommitWrite()
  void GetWritePositions(struct iovec* write_positions, size_t n);
  void CommitWrite(size_t n);

  // NOTE: After called GetReadPositions(),
  // if you indeed read some data from this buffer,
  // you must call CommitRead()
  void GetReadPositions(struct iovec* read_positions, size_t n);
  void CommitRead(size_t n);

  // true means write the 'data' successfully
  // false means there is no enough space to store the data
  // TODO(jinfeng10) resize the buffer dynamically in the future
  bool Write(base::StringPiece data);

  // true means read n bytes of data into 'data' successfully
  // false means there is no enough data
  bool Read(std::string* data, size_t n);
  bool Read(char* data, size_t n);

  void ReadAll(std::string* data) {
    Read(data, size_);
  }

  bool ReadUint32(uint32_t* value);
  // -1 means error
  // 0 means no enough data
  // 1 means ok
  int ReadVarint32(uint32_t* value);

  bool Find(const std::string& delimiters, base::StringPiece* data) {
    return DoFind(delimiters, data);
  }

  bool Find(char delimiter, base::StringPiece* data) {
    base::StringPiece delimiters(&delimiter, 1);
    return DoFind(delimiters, data);
  }

  void Swap(RingBuffer& that) {
    std::swap(buffer_, that.buffer_);
    std::swap(begin_, that.begin_);
    std::swap(end_, that.end_);
    std::swap(size_, that.size_);
    std::swap(capacity_, that.capacity_);
  }

 private:
  // preallocate space for each connection
  char* buffer_;
  int begin_;
  int end_;
  size_t size_;
  size_t capacity_;

  void Reform();
  bool DoFind(base::StringPiece delimiters, base::StringPiece* data);
};

}  // namespace tcp
}  // namespace cnetpp

namespace std {

template<>
inline void swap(cnetpp::tcp::RingBuffer& a, cnetpp::tcp::RingBuffer& b) noexcept {
  a.Swap(b);
}

}  // namespace std

#endif  // CNETPP_TCP_RING_BUFFER_H_

