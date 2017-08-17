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
#include "http_connection.h"
#include "../base/string_utils.h"

namespace cnetpp {
namespace http {

bool HttpConnection::SendPacket(std::shared_ptr<HttpPacket> http_packet) {
  std::string str_packet;
  http_packet->ToString(&str_packet);
  return SendPacket(str_packet);
}

bool HttpConnection::SendPacket(base::StringPiece data) {
  return tcp_connection_->SendPacket(data);
}

bool HttpConnection::OnConnected() {
  if (connected_callback_) {
    connected_callback_(shared_from_this());
  }
  return true;
}

bool HttpConnection::OnClosed() {
  if (!closed_callback_) {
    return true;
  }
  return closed_callback_(shared_from_this());
}

bool HttpConnection::OnReceived() {
  auto& recv_buffer = tcp_connection_->mutable_recv_buffer();
  while (true) {
    switch (receive_status_) {
      case ReceiveStatus::kWaitingHeader: {
        base::StringPiece header;
        if (!recv_buffer.Find("\r\n\r\n", &header)) {
          return true;
        }
        if (!http_packet_->ParseHttpHeaders(header, nullptr)) {
          // TODO(myjfm)
          return false;
        }
        receive_status_ = ReceiveStatus::kWaitingBody;
        recv_buffer.CommitRead(header.length() + 4);
        break;
      }
      case ReceiveStatus::kWaitingBody: {
        int content_length = http_packet_->GetContentLength();
        if (content_length >= 0) {
          if (!recv_buffer.Read(&(http_packet_->mutable_http_body()),
                                content_length)) {
            return true;
          }
        } else {
          // process "Transfer-Encoding: chunked" case
          std::string* chunked = nullptr;
          if (http_packet_->GetHttpHeader("Transfer-Encoding", &chunked) &&
              base::StringPiece(*chunked).ignore_case_equal("chunked")) {
            receive_status_ = ReceiveStatus::kWaitingChunkSize;
            break;
          }
        }
        receive_status_ = ReceiveStatus::kCompleted;
        break;
      }
      case ReceiveStatus::kWaitingChunkSize: {
        base::StringPiece chunk_size_line;
        if (!recv_buffer.Find("\r\n", &chunk_size_line)) {
          return true;  // no enough data
        }
        if (chunk_size_line.size() == 0) {
          // should not happen
          return false;
        }
        if (chunk_size_line[0] == '0') {  // last chunk
          receive_status_ = ReceiveStatus::kWaitingChunkTrailer;
          recv_buffer.CommitRead(chunk_size_line.length() + 2);
          break;
        }
        current_chunk_size_ =
            base::StringUtils::HexStrToInteger(chunk_size_line, nullptr);
        recv_buffer.CommitRead(chunk_size_line.length() + 2);
        receive_status_ = ReceiveStatus::kWaitingChunkData;
        break;
      }
      case ReceiveStatus::kWaitingChunkData: {
        base::StringPiece chunk_size_line;
        if (!recv_buffer.Read(&(http_packet_->mutable_http_body()),
                              current_chunk_size_ + 2)) {  // chunk_data + \r\n
          return true;  // no enough data
        }
        http_packet_->mutable_http_body().erase(
            http_packet_->http_body().length() - 2, 2);
        receive_status_ = ReceiveStatus::kWaitingChunkSize;
        break;
      }
      case ReceiveStatus::kWaitingChunkTrailer: {
        base::StringPiece trailer_line;
        if (!recv_buffer.Find("\r\n", &trailer_line)) {
          return true;
        }
        // just ignore the trailer
        recv_buffer.CommitRead(trailer_line.length() + 2);
        receive_status_ = ReceiveStatus::kCompleted;
        break;
      }
      case ReceiveStatus::kCompleted:
        // call the callback
        if(received_callback_) {
          received_callback_(shared_from_this());
        }
        receive_status_ = ReceiveStatus::kWaitingHeader;
        http_packet_->Reset();
        break;
      default:
        assert(false);
        return false;
    }
  }
  return true;
}

bool HttpConnection::OnSent(bool success) {
  if (!sent_callback_) {
    return true;
  }
  return sent_callback_(success, shared_from_this());
}

}  // namespace http
}  // namespace cnetpp

