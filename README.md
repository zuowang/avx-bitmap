# avx-bitmap
impala@40dfc5751fde:~$ /opt/intel/bin/icpc -march=core-avx2 avxbitmap.cc
impala@40dfc5751fde:~$ ./a.out 
get time: 1090000.000000 -8

impala@40dfc5751fde:~$ /opt/intel/bin/icpc -march=core-avx2 bitmap.cc
impala@40dfc5751fde:~$ ./a.out 
get time: 8240000.000000 0


It shows an 8x faster when rewrite the bitmap with avx/avx2 intrinsics.
