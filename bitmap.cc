#include<immintrin.h>
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#define NUM_ROWS 1024000000
#define BITMAP_SIZE 20480
int main() {
  int* bit_index = new int[NUM_ROWS];
  int buffer[BITMAP_SIZE];
  clock_t finish1 = clock();
  int rows_filtered = 0;
  for (int j = 0; j < NUM_ROWS; ++j) {
    bit_index[j] %= BITMAP_SIZE;
    int64_t word_index = bit_index[j] >> 8;
    bit_index[j] &= 63;
    bool filtered = (buffer[word_index] & (1 << bit_index[j])) != 0;
    rows_filtered += (int)filtered;
  }
  clock_t finish2 = clock();
  printf("time: %f rows filtered: %d\n", (double)(finish2 - finish1), rows_filtered);
  delete [] bit_index;
  return 0;
}
