# counting-chars
Small and stupid benchmarking of ways of counting a char in a big string.

The results, just for the reference (all times are in milliseconds):

| CPU                     | Naive / `-O2` | Naive / `-O3 -march=native` | SSE4.2 | AVX2 |
|:----------------------- | -------------:| ---------------------------:| ------:| ----:|
| Core i7 3930k @ 4.0 GHz |          1060 |                         506 |    255 |    — |
| Core i7 4770            |           921 |                         332 |    253 |  125 |
| Core i7 6700            |           913 |                         317 |    218 |  118 |
