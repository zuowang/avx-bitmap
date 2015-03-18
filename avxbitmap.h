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
#include<immintrin.h>
using namespace std;

namespace impala {

// Bitmap vector utility class.
// TODO: investigate perf.
//  - Precomputed bitmap
//  - Explicit Set/Unset() apis
//  - Bigger words
//  - size bitmap to Mersenne prime.
class AVXBitmap {
 public:
  AVXBitmap(int64_t num_bits) {
    capacity_ = (num_bits + 63) >> 6;
    buffer_ = new int[num_bits];
    size_ = num_bits;
    msize_ = _mm256_set1_ps((float)size_);
    mand_ = _mm256_set1_ps(63.0);
    mzero_ = _mm256_set1_ps(0.0);
  }

  virtual ~AVXBitmap() {
    if (buffer_) delete [] buffer_;
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
  inline __m256 Get(__m256 bit_index) const {
    __m256 f = _mm256_div_ps(bit_index, msize_);
    __m256 m = _mm256_mul_ps(f, msize_);
    __m256 a = _mm256_sub_ps(bit_index, m);
    __m256i word_index = _mm256_srai_epi32(_mm256_cvtps_epi32(a), 8);
    a = _mm256_and_ps(a, mand_);
    __m256 c = _mm256_i32gather_ps((float const*)buffer_, word_index, 4);
    __m256i di = _mm256_slli_epi32(_mm256_cvtps_epi32(a), 1);
    __m256 e = _mm256_and_ps(c, _mm256_cvtepi32_ps(di));
    return _mm256_cmp_ps(e, mzero_, _CMP_EQ_OQ);
  }

  // Bitwise ANDs the src bitmap into this one.
  void And(const AVXBitmap* src) {
    for (int i = 0; i < capacity_; ++i) {
      buffer_[i] &= src->buffer_[i];
    }
  }

  // Bitwise ORs the src bitmap into this one.
  void Or(const AVXBitmap* src) {
    for (int i = 0; i < capacity_; ++i) {
      buffer_[i] |= src->buffer_[i];
    }
  }

  void SetAllBits(bool b) {
    memset(&buffer_[0], 255 * b, capacity_ * sizeof(int));
  }

  int64_t size() const { return size_; }

 private:
  int* buffer_;
  int64_t size_;
  int64_t capacity_;
  __m256 msize_;
  __m256 mand_;
  __m256 mzero_;
};

}

#endif
