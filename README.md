# counting-chars
Small and stupid benchmarking of ways of counting a char in a big string.

The results, just for the reference (all times are in milliseconds, gcc 9.2, input file size is about 1.8 GB):

| CPU                     | Naive / `-O2` | Naive / `-O3 -march=native` | SSE4.2 | AVX2 |
|:----------------------- | -------------:| ---------------------------:| ------:| ----:|
| Core i7 3930k @ 4.0 GHz |          1060 |                         506 |    255 |    — |
| Core i7 4770            |           921 |                         332 |    253 |  125 |
| Core i7 6700            |           913 |                         317 |    218 |  118 |
| Ryzen 7 3700X           |           863 |                         352 |    126 |  116 |
