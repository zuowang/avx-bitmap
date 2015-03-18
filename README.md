# avx-bitmap
Compile and run the test with below command:
$ /opt/intel/bin/icpc  bitmap.cc
$ ./a.out 
time: 2870000.000000 rows filtered: 0

$ /opt/intel/bin/icpc -march=core-avx2 bitmap.cc
$ ./a.out 
time: 1560000.000000 rows filtered: 0

$ /opt/intel/bin/icpc -march=core-avx2 test2.cc
$ ./a.out 
1080000.000000

It shows an 1.84x faster when compiled original bitmap with ICC(Intel C++ Compiler) with avx2 enabled.
It shows an 2.66x faster when rewrite the bitmap with avx/avx2 intrinsics.
