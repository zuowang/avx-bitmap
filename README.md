# avx-bitmap
```
impala@40dfc5751fde:~/avx-bitmap$ /opt/intel/bin/icpc -march=core-avx2 bitmap.cc
impala@40dfc5751fde:~/avx-bitmap$ ./a.out
time for Get operation: 8250000.000000 rows filtered:0
time for Or operation: 40000.000000
time for And operation: 50000.000000
time for Set operation: 9500000.000000
```
```
impala@40dfc5751fde:~/avx-bitmap$ /opt/intel/bin/icpc -march=core-avx2 avxbitmap.cc
impala@40dfc5751fde:~/avx-bitmap$ ./a.out
time for Get operation: 1160000.000000 rows filtered:-8
time for Or operation: 10000.000000
time for And operation: 20000.000000
time for Set operation: 1700000.000000
```



bitmap get mod:       
                       Function     Rate (iters/ms)          Comparison
----------------------------------------------------------------------
                        Scalar                  64                  1X
                          AVX2               189.7              2.963X


