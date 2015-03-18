#include<immintrin.h>
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#define NUM_ROWS 1024000000
#define BITMAP_SIZE 20480
int main() {
  int* bit_index = new int[NUM_ROWS];
  int buffer[BITMAP_SIZE];
  float size2 = 20480.0;
  float fact2 = 63.0;
  clock_t finish1=clock();
  __m256 f2 = _mm256_broadcast_ss(&size2);
  __m256i b = _mm256_cvtps_epi32(_mm256_broadcast_ss(&fact2));
  float zero = 0.0;
  __m256i zerobc_ = _mm256_cvtps_epi32(_mm256_broadcast_ss(&zero));

  __m256i rows_filtedm;
  int rows_filtered = 0;
  float mf2 = 1.0;
  __m256i mask2 = _mm256_cvtps_epi32(_mm256_broadcast_ss(&mf2));
  for (int k = 0; k < NUM_ROWS; k+=8) {
    __m256 a = _mm256_loadu_ps((float const*)(bit_index + k));
    // a%size
    __m256 f = _mm256_div_ps(a, f2);
    __m256 m = _mm256_mul_ps(f, f2);
    a  = _mm256_sub_ps(a, m);
    __m256i ai = _mm256_cvtps_epi32(a);
    __m256i word_index = _mm256_srai_epi32(ai, 8);
    ai = _mm256_and_si256(ai, b);
    __m256i ci = _mm256_i32gather_epi32(buffer, word_index, 4);
    __m256i di = _mm256_slli_epi32(ai, 1);
    __m256i ei = _mm256_and_si256(ci, di);
    __m256i fi = _mm256_cmpeq_epi32(ei, zerobc_);
    rows_filtedm = _mm256_add_epi32(rows_filtedm, fi);
  }
  int tmp[8];
  _mm256_storeu_ps((float*)tmp, _mm256_cvtepi32_ps(rows_filtedm));
  for (int i = 0; i < 8; ++i) rows_filtered += tmp[i];
  clock_t finish2=clock();
  printf("%f\n", (double)(finish2 - finish1));
  delete [] bit_index;
  return 0;
}
