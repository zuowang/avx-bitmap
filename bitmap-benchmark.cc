/ Copyright 2015 Cloudera Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "util/benchmark.h"
#include "util/cpu-info.h"
#include "util/bitmap.h"

#include "common/names.h"

using namespace impala;


struct TestData {
  TestData(int num_bits) : bm(num_bits) {}
  Bitmap bm;
  vector<uint32_t> bit_index;
  vector<uint32_t> bit_index_mod;
  vector<bool> result;
};

void TestScalarGetMod(int batch_size, void* d) {
  TestData* data = reinterpret_cast<TestData*>(d);
  for (int j = 0; j < batch_size; ++j) {
    for (int bit_idx = 0; bit_idx < data->bit_index_mod.size(); ++bit_idx) {
      data->result.push_back(data->bm.Get<true>(data->bit_index_mod[bit_idx]));
    }
  }
}

void TestAVX2GetMod(int batch_size, void* d) {
  TestData* data = reinterpret_cast<TestData*>(d);
  uint32_t result[] = {0, 0, 0, 0, 0, 0, 0, 0};
  for (int j = 0; j < batch_size; ++j) {
    for (int64_t bit_idx = 0; bit_idx < data->bit_index_mod.size(); bit_idx += 8) {
      data->bm.Get<true>(&data->bit_index_mod[bit_idx], result);
      data->result.push_back(result[0] != 0);
      data->result.push_back(result[1] != 0);
      data->result.push_back(result[2] != 0);
      data->result.push_back(result[3] != 0);
      data->result.push_back(result[4] != 0);
      data->result.push_back(result[5] != 0);
      data->result.push_back(result[6] != 0);
      data->result.push_back(result[7] != 0);
    }
  }
}
int main(int argc, char **argv) {
  CpuInfo::Init();
  cout << Benchmark::GetMachineInfo() << endl;

  TestData data(1024);
  data.bm.SetAllBits(false);

  for (int64_t bit_idx = 0; bit_idx < 1024; ++bit_idx) {
    data.bit_index_mod.push_back(bit_idx);
    data.bm.Set<true>(bit_idx, bit_idx % 2 == 0);
  }


  Benchmark mod_suite("bitmap get mod");
  mod_suite.AddBenchmark("Scalar", TestScalarGetMod, &data);
  mod_suite.AddBenchmark("AVX2", TestAVX2GetMod, &data);
  cout << mod_suite.Measure() << endl;
  return 0;
}
