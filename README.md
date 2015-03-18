# avx-bitmap
<p>impala@40dfc5751fde:~/avx-bitmap$ /opt/intel/bin/icpc -march=core-avx2 bitmap.cc</p>
<p>impala@40dfc5751fde:~/avx-bitmap$ ./a.out</p>
<p>time for Get operation: 8250000.000000 rows filtered:0</p>
<p>time for Or operation: 40000.000000</p>
<p>time for And operation: 50000.000000</p>
<p>time for Set operation: 9500000.000000</p>

<p>mpala@40dfc5751fde:~/avx-bitmap$ /opt/intel/bin/icpc -march=core-avx2 avxbitmap.cc</p>
<p>impala@40dfc5751fde:~/avx-bitmap$ ./a.out</p> 
<p>time for Get operation: 1160000.000000 rows filtered:-8</p>
<p>time for Or operation: 10000.000000</p>
<p>time for And operation: 20000.000000</p>
<p>time for Set operation: 1700000.000000</p>

<p>------------------------</p>
<p>| Operation | x faster |</p>
<p>------------------------</p>
<p>|   Get     |   7.1    |</p>
<p>------------------------</p>
<p>|   Or      |   4.0    |</p>
<p>------------------------</p>
<p>|   And     |   3.5    |</p>
<p>------------------------</p>
<p>|   Set     |   5.6    |</p>
<p>------------------------</p>

