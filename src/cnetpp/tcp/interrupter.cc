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
#include <cnetpp/tcp/interrupter.h>
#if defined(linux) || defined(__linux) || defined(__linux__)
#include <cnetpp/tcp/eventfd_interrupter_impl.h>
#else
#include <cnetpp/tcp/pipe_interrupter_impl.h>
#endif

namespace cnetpp {
namespace tcp {

std::unique_ptr<Interrupter> Interrupter::New() {
#if defined(linux) || defined(__linux) || defined(__linux__)
  return std::unique_ptr<Interrupter>(new EventfdInterrupterImpl());
#else
  return std::unique_ptr<Interrupter>(new PipeInterrupterImpl());
#endif
}

}  // namespace tcp
}  // namespace cnetpp

