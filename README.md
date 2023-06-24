# counting-chars
Small and stupid benchmarking of ways of counting a char in a big string.

The results, just for the reference (all times are in milliseconds, gcc 9.2, input file size is about 1.8 GB):

| CPU                     | Naive / `-O2` | Naive / `-O3 -march=native` | SSE4.2 | AVX2 |
|:----------------------- | -------------:| ---------------------------:| ------:| ----:|
| Core i7 3930k @ 4.0 GHz |          1060 |                         506 |    255 |    — |
| Core i7 4770            |           921 |                         332 |    253 |  125 |
| Core i7 6700            |           913 |                         317 |    218 |  118 |


## Generating test set

We use King James version of the bible from The Large Corpus [here](https://corpus.canterbury.ac.nz/descriptions/),
concatenated 500 times with itself:
```shell
wget http://corpus.canterbury.ac.nz/resources/large.tar.gz
tar xf large.tar.gz bible.txt
for i in `seq 1 500`; do cat bible.txt >> bible_large.txt; done
```
