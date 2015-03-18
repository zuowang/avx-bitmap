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

#include "bitmap.h"
#include <time.h>
using namespace impala;

#define NUM_ROWS 1024000000
#define BITMAP_SIZE 20480
int main() {
  int* rows = new int[NUM_ROWS];
  Bitmap bm(BITMAP_SIZE);
  clock_t start = clock();
  int rows_filted = 0;
  for (int i = 0; i < NUM_ROWS; ++i) {
    if (bm.Get<true>(rows[i])) ++rows_filted;
  }
  clock_t end = clock();
  printf("get time: %f %d\n", (double)(end - start), rows_filted);
  delete [] rows;
  return 0;
}
