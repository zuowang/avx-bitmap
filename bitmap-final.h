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

#include "util/bit-util.h"

#include <immintrin.h>

namespace impala {

/// Bitmap vector utility class.
/// TODO: investigate perf.
///  - Precomputed bitmap
///  - Explicit Set/Unset() apis
///  - Bigger words
///  - size bitmap to Mersenne prime.
class Bitmap {
 public:
  Bitmap(int64_t num_bits) {
    DCHECK_GE(num_bits, 0);
    buffer_.resize(BitUtil::RoundUpNumi64(num_bits));
    num_bits_ = num_bits;
    init(num_bits);
  }

  inline uint32_t bit_scan_reverse (uint32_t a) {
      uint32_t r;
      __asm("bsrl %1, %0" : "=r"(r) : "r"(a) : );
      return r;
  }

  void init(uint32_t d) {                                 // Set or change divisor, calculate parameters
    uint32_t L, L2, sh1, sh2, m;
    switch (d) {
    case 0:
        m = sh1 = sh2 = 0;                         // provoke error for d = 0
        break;
    case 1:
        m = 1; sh1 = sh2 = 0;                          // parameters for d = 1
        break;
    case 2:
        m = 1; sh1 = 1; sh2 = 0;                       // parameters for d = 2
        break;
    default:                                           // general case for d > 2
        L  = bit_scan_reverse(d-1)+1;                  // ceil(log2(d))
        L2 = L < 32 ? 1 << L : 0;                      // 2^L, overflow to 0 if L = 32
        m  = 1 + uint32_t((uint64_t(L2 - d) << 32) / d); // multiplier
        sh1 = 1;  sh2 = L - 1;                         // shift counts
    }
    multiplier = m;
    shift1     = sh1;
    shift2     = sh2;
    L1 = L;
  }
  /// Resize bitmap and set all bits to zero.
  void Reset(int64_t num_bits) {
    DCHECK_GE(num_bits, 0);
    buffer_.resize(BitUtil::RoundUpNumi64(num_bits));
    num_bits_ = num_bits;
    SetAllBits(false);
  }

  /// Compute memory usage of a bitmap, not including the Bitmap object itself.
  static int64_t MemUsage(int64_t num_bits) {
    DCHECK_GE(num_bits, 0);
    return BitUtil::RoundUpNumi64(num_bits) * sizeof(int64_t);
  }

  /// Compute memory usage of this bitmap, not including the Bitmap object itself.
  int64_t MemUsage() const { return MemUsage(num_bits_); }

  /// Sets the bit at 'bit_index' to v. If mod is true, this
  /// function will first mod the bit_index by the bitmap size.
  template<bool mod>
  void Set(int64_t bit_index, bool v) {
    if (mod) bit_index %= num_bits();
    int64_t word_index = bit_index >> NUM_OFFSET_BITS;
    bit_index &= BIT_INDEX_MASK;
    DCHECK_LT(word_index, buffer_.size());
    if (v) {
      buffer_[word_index] |= (1LL << bit_index);
    } else {
      buffer_[word_index] &= ~(1LL << bit_index);
    }
  }
  template<bool mod>
  __attribute__ ((target("default")))
  bool Get(int64_t bit_index) const {
    if (mod) {
      int64_t m = (bit_index * multiplier) >> 32;
      int64_t q = (m + bit_index) >> L1;
      bit_index = bit_index - q * num_bits();
//      bit_index %= num_bits();
    }
    int64_t word_index = bit_index >> NUM_OFFSET_BITS;
    bit_index &= BIT_INDEX_MASK;
    DCHECK_LT(word_index, buffer_.size());
    return (buffer_[word_index] & (1LL << bit_index)) != 0;
  }

#ifndef IR_COMPILE
  template<bool mod>
  __attribute__ ((target("avx2")))
  inline void Get(uint32_t* bit_index, uint32_t* result) const {
    static __m256i m_bit_mask = _mm256_set1_epi32(31);
    static __m256i m_one = _mm256_set1_epi32(1);
    __m256i m_bit_index;
    if (mod) {
    __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(bit_index));
    static __m256i m   = _mm256_set1_epi32(multiplier);       // broadcast multiplier
    __m256i t1  = _mm256_mul_epu32(a,m);                   // 32x32->64 bit unsigned multiplication of even elements of a
    __m256i t2  = _mm256_srli_epi64(t1,32);                // high dword of even numbered results
    __m256i t3  = _mm256_srli_epi64(a,32);                 // get odd elements of a into position for multiplication
    __m256i t4  = _mm256_mul_epu32(t3,m);                  // 32x32->64 bit unsigned multiplication of odd elements
    static __m256i t5  = _mm256_set_epi32(-1, 0, -1, 0, -1, 0, -1, 0);      // mask for odd elements
    __m256i t7  = _mm256_blendv_epi8(t2,t4,t5);            // blend two results
    __m256i t8  = _mm256_sub_epi32(a,t7);                  // subtract
    __m256i t9  = _mm256_srli_epi32(t8,shift1);          // shift right logical
    __m256i t10 = _mm256_add_epi32(t7,t9);                 // add
    __m256i t11 = _mm256_srli_epi32(t10,shift2);         // shift right logical
    static __m256i m_num_bits = _mm256_set1_epi32(num_bits_);
    __m256i t12 = _mm256_mullo_epi32(t11,m_num_bits);
    m_bit_index = _mm256_sub_epi32(a,t12);
    } else {
      m_bit_index = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(bit_index));
    }


    __m256i m_word_index = _mm256_srli_epi32(m_bit_index, 5);
    __m256i m_words = _mm256_i32gather_epi32(
        reinterpret_cast<const int*>(&buffer_[0]), m_word_index, 4);
    m_bit_index = _mm256_and_si256(m_bit_index, m_bit_mask);
    __m256i m_masks = _mm256_sllv_epi32(m_one, m_bit_index);
    __m256i m_result = _mm256_and_si256(m_words, m_masks);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(result), m_result);
  }
#endif

  /// Bitwise ANDs the src bitmap into this one.
  void And(const Bitmap* src) {
    DCHECK_EQ(num_bits(), src->num_bits());
    for (int i = 0; i < buffer_.size(); ++i) {
      buffer_[i] &= src->buffer_[i];
    }
  }

  /// Bitwise ORs the src bitmap into this one.
  void Or(const Bitmap* src) {
    DCHECK_EQ(num_bits(), src->num_bits());
    for (int i = 0; i < buffer_.size(); ++i) {
      buffer_[i] |= src->buffer_[i];
    }
  }

  void SetAllBits(bool b) {
    memset(&buffer_[0], 255 * b, buffer_.size() * sizeof(uint64_t));
  }
  int64_t num_bits() const { return num_bits_; }

  /// If 'print_bits' prints 0/1 per bit, otherwise it prints the int64_t value.
  std::string DebugString(bool print_bits);

 private:
  std::vector<uint64_t> buffer_;
  int64_t num_bits_;
  uint32_t multiplier;                                    // multiplier used in fast division
  uint32_t shift1;                                        // shift count 1 used in fast division
  uint32_t shift2;                                        // shift count 2 used in fast division
  uint32_t L1;
  /// Used for bit shifting and masking for the word and offset calculation.
  static const int64_t NUM_OFFSET_BITS = 6;
  static const int64_t BIT_INDEX_MASK = 63;
};

}

#endif
