// Copyright 2014 Cloudera Inc.
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

#include "avxbitmap.h"
#include <time.h>
using namespace impala;

#define NUM_ROWS 1024000000
#define BITMAP_SIZE 20480
#define BIG_BITMAP_SIZE 2048000000
int main() {
  AVXBitmap bm(BITMAP_SIZE);
  float* rows = new float[NUM_ROWS];
  clock_t start = clock();
  int rows_filted = 0;
  __m256 rows_filtedm = _mm256_set1_ps(0);;
  for (int i = 0; i < NUM_ROWS; i+=8) {
    rows_filtedm = _mm256_add_ps(rows_filtedm,
        bm.Get<true>(_mm256_loadu_ps((float const*)(rows + i))));
  }
  int tmp[8];
  _mm256_storeu_ps((float*)tmp, rows_filtedm);
  for (int i = 0; i < 8; ++i) rows_filted += tmp[i];
  clock_t end = clock();
  printf("time for Get operation: %f rows filtered:%d\n", (double)(end - start),
      rows_filted);
  delete [] rows;

  AVXBitmap bm1(BIG_BITMAP_SIZE);
  AVXBitmap bm2(BIG_BITMAP_SIZE);
  start = clock();
  bm1.Or(&bm2);
  end = clock();
  printf("time for Or operation: %f\n", (double)(end - start));

  start = clock();
  bm1.And(&bm2);
  end = clock();
  printf("time for And operation: %f\n", (double)(end - start));

  __m256 bit_index;
  __m256i mask;
  start = clock();
  for (int i = 0; i < NUM_ROWS; i+=8) {
    bm.Set<true>(bit_index, mask);
  }
  end = clock();
  printf("time for Set operation: %f\n", (double)(end - start));

  return 0;
}
