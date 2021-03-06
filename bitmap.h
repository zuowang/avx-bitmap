// Copyright 2012 Cloudera Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifndef IMPALA_UTIL_BITMAP_H
#define IMPALA_UTIL_BITMAP_H
#include <vector>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
using namespace std;

namespace impala {

// Bitmap vector utility class.
// TODO: investigate perf.
//  - Precomputed bitmap
//  - Explicit Set/Unset() apis
//  - Bigger words
//  - size bitmap to Mersenne prime.
class Bitmap {
 public:
  Bitmap(int64_t num_bits) {
    buffer_.resize((num_bits + 63) >> 6);
    size_ = num_bits;
  }

  // Sets the bit at 'bit_index' to v. If mod is true, this
  // function will first mod the bit_index by the bitmap size.
  template<bool mod>
  void Set(int64_t bit_index, bool v) {
    if (mod) bit_index %= size();
    int64_t word_index = bit_index >> 8;
    bit_index &= 63;
    if (v) {
      buffer_[word_index] |= (1 << bit_index);
    } else {
      buffer_[word_index] &= ~(1 << bit_index);
    }
  }

  // Returns true if the bit at 'bit_index' is set. If mod is true, this
  // function will first mod the bit_index by the bitmap size.
  template<bool mod>
  inline bool Get(int64_t bit_index) const {
    if (mod) bit_index %= size();
    int64_t word_index = bit_index >> 8;
    bit_index &= 63;
    return (buffer_[word_index] & (1 << bit_index)) != 0;
  }

  // Bitwise ANDs the src bitmap into this one.
  void And(const Bitmap* src) {
    for (int i = 0; i < buffer_.size(); ++i) {
      buffer_[i] &= src->buffer_[i];
    }
  }

  // Bitwise ORs the src bitmap into this one.
  void Or(const Bitmap* src) {
    for (int i = 0; i < buffer_.size(); ++i) {
      buffer_[i] |= src->buffer_[i];
    }
  }

  void SetAllBits(bool b) {
    memset(&buffer_[0], 255 * b, buffer_.size() * sizeof(uint64_t));
  }

  int64_t size() const { return size_; }

 private:
  std::vector<uint64_t> buffer_;
  int64_t size_;
};

}

#endif
