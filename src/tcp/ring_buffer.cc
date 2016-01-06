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
#include "ring_buffer.h"

#include <assert.h>
#include <sys/uio.h>

#include <string.h>
#include <string>
#include <vector>

namespace cnetpp {
namespace tcp {

bool RingBuffer::Resize(size_t new_size) {
  if (new_size < size_) {
    return false;
  }
  char* tmp = new char[new_size];
  if (size_ > 0) {
    if (end_ <= begin_) {
      // readable data is splited into two slices
      ::memcpy(tmp, buffer_ + begin_, capacity_ - begin_);
      ::memcpy(tmp + capacity_ - begin_, buffer_, end_);
    } else {
      ::memcpy(tmp, buffer_ + begin_, end_ - begin_);
    }
  }
  delete [] buffer_;
  buffer_ = tmp;
  capacity_ = new_size;
  begin_ =  0;
  end_ = size_;
  return true;
}

void RingBuffer::GetWritePositions(struct iovec* write_positions, size_t n) {
  assert(write_positions);
  assert(n == 2);

  if (Full()) {
    // buffer is full, no extra writable space to store new data
    write_positions[0].iov_len = 0;
    write_positions[1].iov_len = 0;
    return;
  }

  if (end_ >= begin_) {
    // writable space is splited into two sub-spaces
    write_positions[0].iov_base = buffer_ + end_;
    write_positions[0].iov_len = capacity_ - end_;
    write_positions[1].iov_base = buffer_;
    write_positions[1].iov_len = begin_;
  } else {
    // writable space is continuous
    write_positions[0].iov_base = buffer_ + end_;
    write_positions[0].iov_len = begin_ - end_;
    write_positions[1].iov_len = 0;
  }
}

void RingBuffer::CommitWrite(size_t n) {
  assert(n <= capacity_ - size_);
  size_ += n;

  if (end_ >= begin_) {
    if (n < capacity_ - end_) {
      end_ += n;
    } else {
      end_ = n + end_ - capacity_;
    }
  } else {
    end_ += n;
  }
}

void RingBuffer::GetReadPositions(struct iovec* read_positions, size_t n) {
  assert(read_positions);
  assert(n == 2);

  if (Empty()) {
    // buffer is empty, no extra readable data
    read_positions[0].iov_len = 0;
    read_positions[1].iov_len = 0;
    return;
  }

  if (end_ <= begin_) {
    // readable data is splited into two slices
    read_positions[0].iov_base = buffer_ + begin_;
    read_positions[0].iov_len = capacity_ - begin_;
    read_positions[1].iov_base = buffer_;
    read_positions[1].iov_len = end_;
  } else {
    // readable data is continuous
    read_positions[0].iov_base = buffer_ + begin_;
    read_positions[0].iov_len = end_ - begin_;
    read_positions[1].iov_len = 0;
  }
}

void RingBuffer::CommitRead(size_t n) {
  assert(n <= size_);
  size_ -= n;

  if (end_ <= begin_) {
    if (n < capacity_ - begin_) {
      begin_ += n;
    } else {
      begin_ = n + begin_ - capacity_;
    }
  } else {
    begin_ += n;
  }
}

bool RingBuffer::Write(base::StringPiece data) {
  if (data.length() == 0) {
    return true;
  }

  if (data.length() > capacity_ - size_) {
    return false;
  }

  size_ += data.length();

  if (end_ >= begin_) {
    // writable space is splited into two sub-spaces
    if (data.length() <= capacity_ - end_) {
      ::memcpy(buffer_ + end_, data.data(), data.length());
      end_ += data.length();
    } else {
      ::memcpy(buffer_ + end_, data.data(), capacity_ - end_);
      ::memcpy(buffer_,
               data.data() + capacity_ - end_,
               data.length() + end_ - capacity_);
      end_ = data.length() + end_ - capacity_;
    }
  } else {
    // writable space is continuous
    ::memcpy(buffer_ + end_, data.data(), data.length());
    end_ += data.length();
  }
  return true;
}

bool RingBuffer::Read(std::string* data, size_t n) {
  assert(data);
  if (n == 0) {
    return true;
  }

  if (size_ < n) {
    return false;
  }

  data->reserve(data->size() + n);
  size_ -= n;

  if (end_ <= begin_) {
    // readable data is splited into two slices
    if (n <= capacity_ - begin_) {
      data->append(buffer_ + begin_, n);
      begin_ += n;
    } else {
      data->append(buffer_ + begin_, capacity_ - begin_);
      data->append(buffer_, n + begin_ - capacity_);
      begin_ = n + begin_ - capacity_;
    }
  } else {
    // readable data is continuous
    data->append(buffer_ + begin_, n);
    begin_ += n;
  }
  return true;
}

bool RingBuffer::DoFind(base::StringPiece delimiters, base::StringPiece* data) {
  if (size_ <= 0) {
    return false;
  }
  if (end_ <= begin_) {
    Reform();
  }
  
  base::StringPiece buf(buffer_ + begin_, size_);
  base::StringPiece::size_type idx = buf.find(delimiters);
  if (idx == base::StringPiece::npos) {
    return false;
  }
  data->set(buffer_ + begin_, idx);
  return true;
}

void RingBuffer::Reform() {
  if (end_ <= 2 * begin_ - static_cast<int>(capacity_)) {
    ::memmove(buffer_ + capacity_ - begin_, buffer_, end_);
    ::memcpy(buffer_, buffer_ + begin_, capacity_ - begin_);
  } else {
    if (end_ <= static_cast<int>(capacity_) - begin_) {
      char* tmp = new char[end_];
      ::memcpy(tmp, buffer_, end_);
      ::memcpy(buffer_, buffer_ + begin_, capacity_ - begin_);
      ::memcpy(buffer_ + capacity_ - begin_, tmp, end_);
      delete [] tmp;
    } else {
      char* tmp = new char[capacity_ - begin_];
      ::memcpy(tmp, buffer_ + begin_, capacity_ - begin_);
      ::memmove(buffer_ + capacity_ - begin_, buffer_, end_);
      ::memcpy(buffer_, tmp, capacity_ - begin_);
      delete [] tmp;
    }
  }
  begin_ = 0;
  end_ = size_;
}

}  // namespace tcp
}  // namespace cnetpp

