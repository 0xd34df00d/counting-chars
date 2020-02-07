#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <x86intrin.h>
#include <nmmintrin.h>

unsigned long fps_count_naive(unsigned char *str, unsigned long len, unsigned char w) {
    unsigned long c;
    for (c = 0; len-- != 0; ++str)
        if (*str == w)
            ++c;
    return c;
}

__attribute__((target("sse4.2")))
unsigned long fps_count_cmpestrm(unsigned char *str, unsigned long len, unsigned char w) {
    __m128i pat = _mm_set1_epi8(w);

    const int mode = _SIDD_SBYTE_OPS | _SIDD_CMP_EQUAL_EACH;

    unsigned long res = 0;

    size_t i = 0;

    for (; i < len && (intptr_t)(str + i) % 64; ++i) {
        res += str[i] == w;
    }

    for (unsigned long end = len - 128; i < end; i += 128) {
        __m128i p0 = _mm_load_si128((const __m128i*)(str + i + 16 * 0));
        __m128i p1 = _mm_load_si128((const __m128i*)(str + i + 16 * 1));
        __m128i p2 = _mm_load_si128((const __m128i*)(str + i + 16 * 2));
        __m128i p3 = _mm_load_si128((const __m128i*)(str + i + 16 * 3));
        __m128i p4 = _mm_load_si128((const __m128i*)(str + i + 16 * 4));
        __m128i p5 = _mm_load_si128((const __m128i*)(str + i + 16 * 5));
        __m128i p6 = _mm_load_si128((const __m128i*)(str + i + 16 * 6));
        __m128i p7 = _mm_load_si128((const __m128i*)(str + i + 16 * 7));
        __m128i r0 = _mm_cmpestrm(p0, 16, pat, 16, mode);
        __m128i r1 = _mm_cmpestrm(p1, 16, pat, 16, mode);
        __m128i r2 = _mm_cmpestrm(p2, 16, pat, 16, mode);
        __m128i r3 = _mm_cmpestrm(p3, 16, pat, 16, mode);
        __m128i r4 = _mm_cmpestrm(p4, 16, pat, 16, mode);
        __m128i r5 = _mm_cmpestrm(p5, 16, pat, 16, mode);
        __m128i r6 = _mm_cmpestrm(p6, 16, pat, 16, mode);
        __m128i r7 = _mm_cmpestrm(p7, 16, pat, 16, mode);
        res += _popcnt64(_mm_extract_epi64(r0, 0));
        res += _popcnt64(_mm_extract_epi64(r1, 0));
        res += _popcnt64(_mm_extract_epi64(r2, 0));
        res += _popcnt64(_mm_extract_epi64(r3, 0));
        res += _popcnt64(_mm_extract_epi64(r4, 0));
        res += _popcnt64(_mm_extract_epi64(r5, 0));
        res += _popcnt64(_mm_extract_epi64(r6, 0));
        res += _popcnt64(_mm_extract_epi64(r7, 0));
    }

    for (; i < len; ++i) {
        res += str[i] == w;
    }

    return res;
}

__attribute__((target("avx2")))
unsigned long fps_count_256(unsigned char *str, unsigned long len, unsigned char w) {
    __m256i pat = _mm256_set1_epi8(w);

    unsigned long prefix, res = 0;

    size_t i = 0;

    for (; i < len && (intptr_t)(str + i) % 64; ++i) {
        prefix += str[i] == w;
    }

    for (unsigned long end = len - 32; i < end; i += 32) {
        __m256i p0 = _mm256_load_si256((const __m256i*)(str + i));
        __m256i r0 = _mm256_cmpeq_epi8(p0, pat);
        res += _popcnt64(_mm256_extract_epi64(r0, 0));
        res += _popcnt64(_mm256_extract_epi64(r0, 1));
        res += _popcnt64(_mm256_extract_epi64(r0, 2));
        res += _popcnt64(_mm256_extract_epi64(r0, 3));
    }

    res /= 8;
    res += prefix;

    for (; i < len; ++i) {
        res += str[i] == w;
    }

    return res;
}

int main(int argc, char **argv) {
    if (argc != 3)
        return 1;

    const int iters = 10;
    unsigned char w = ' ';

    unsigned long (*ptr) (unsigned char*, unsigned long, unsigned char) = NULL;

    switch(argv[2][0]) {
    case 'n':
        ptr = &fps_count_naive;
        break;
    case 'c':
        ptr = &fps_count_cmpestrm;
        break;
    case '2':
        ptr = &fps_count_256;
        break;
    }

    const int fd = open(argv[1], O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);

    unsigned char *str = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    struct tms buf;
    times(&buf);
    const clock_t start = buf.tms_utime;

    unsigned long cnt = 0;
    for (int i = 0; i < iters; ++i)
        cnt += (*ptr)(str, sb.st_size, w);

    times(&buf);
    const clock_t end = buf.tms_utime;

    printf("%lu %ld\n", cnt / iters, (end - start) * 1000 / sysconf(_SC_CLK_TCK) / iters);
}
