# counting-chars
Small and stupid benchmarking of ways of counting a char in a big string.

## `-O2`

| CPU                        | Naive | SSE4.2 | AVX2 |
|:-------------------------- | -----:| ------:| ----:|
| Ryzen 9 7950X3D (gcc 13)   |   712 |     88 |   56 |
| Ryzen 9 7950X3D (clang 16) |   464 |     83 |   45 |

## `-O3 -march=native`

| CPU                        | Naive | SSE4.2 | AVX2 |
|:-------------------------- | -----:| ------:| ----:|
| Ryzen 9 7950X3D (gcc 13)   |    76 |     83 |   70 |
| Ryzen 9 7950X3D (clang 16) |    62 |     81 |   42 |

## Old ones

Older results, with gcc 9.2 and a somewhat different hardware:

| CPU                     | Naive / `-O2` | Naive / `-O3 -march=native` | SSE4.2 | AVX2 |
|:----------------------- | -------------:| ---------------------------:| ------:| ----:|
| Core i7 3930k @ 4.0 GHz |          1060 |                         506 |    255 |    — |
| Core i7 4770            |           921 |                         332 |    253 |  125 |
| Core i7 6700            |           913 |                         317 |    218 |  118 |
| Ryzen 7 3700X           |           863 |                         352 |    126 |  116 |


## Generating test input

We use King James version of the bible from The Large Corpus [here](https://corpus.canterbury.ac.nz/descriptions/),
concatenated 500 times with itself:
```shell
wget http://corpus.canterbury.ac.nz/resources/large.tar.gz
tar xf large.tar.gz bible.txt
for i in `seq 1 500`; do cat bible.txt >> bible_large.txt; done
```
