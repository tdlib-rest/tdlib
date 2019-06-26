//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2019
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/mtproto/TlsReaderByteFlow.h"

#include "td/utils/Status.h"

namespace td {

void TlsReaderByteFlow::loop() {
  while (true) {
    if (input_->size() < 5) {
      set_need_size(5);
      return;
    }

    auto it = input_->clone();
    uint8 buf[5];
    it.advance(5, MutableSlice(buf, 5));
    if (Slice(buf, 3) != Slice("\x17\x03\x03")) {
      close_input(td::Status::Error("Invalid bytes at the beginning of a packet (emulated tls)"));
      return;
    }
    size_t len = (buf[3] << 8) | buf[4];
    if (len > (1 << 14)) {
      close_input(td::Status::Error("Packet lenght is too big (emulated tls)"));
      return;
    }

    if (it.size() < len) {
      set_need_size(5 + len);
      return;
    }

    output_.append(it.cut_head(len));
    *input_ = std::move(it);
  }
}

}  // namespace td
