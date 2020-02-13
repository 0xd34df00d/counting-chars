#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <threads.h>

#include <x86intrin.h>
#include <nmmintrin.h>
#include <cpuid.h>

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
unsigned long fps_count_avx2(unsigned char *str, unsigned long len, unsigned char w) {
    __m256i pat = _mm256_set1_epi8(w);

    unsigned long prefix = 0, res = 0;

    size_t i = 0;

    for (; i < len && (intptr_t)(str + i) % 64; ++i) {
        prefix += str[i] == w;
    }

    for (unsigned long end = len - 128; i < end; i += 128) {
        __m256i p0 = _mm256_load_si256((const __m256i*)(str + i + 32 * 0));
        __m256i p1 = _mm256_load_si256((const __m256i*)(str + i + 32 * 1));
        __m256i p2 = _mm256_load_si256((const __m256i*)(str + i + 32 * 2));
        __m256i p3 = _mm256_load_si256((const __m256i*)(str + i + 32 * 3));
        __m256i r0 = _mm256_cmpeq_epi8(p0, pat);
        __m256i r1 = _mm256_cmpeq_epi8(p1, pat);
        __m256i r2 = _mm256_cmpeq_epi8(p2, pat);
        __m256i r3 = _mm256_cmpeq_epi8(p3, pat);
        res += _popcnt64(_mm256_extract_epi64(r0, 0));
        res += _popcnt64(_mm256_extract_epi64(r0, 1));
        res += _popcnt64(_mm256_extract_epi64(r0, 2));
        res += _popcnt64(_mm256_extract_epi64(r0, 3));
        res += _popcnt64(_mm256_extract_epi64(r1, 0));
        res += _popcnt64(_mm256_extract_epi64(r1, 1));
        res += _popcnt64(_mm256_extract_epi64(r1, 2));
        res += _popcnt64(_mm256_extract_epi64(r1, 3));
        res += _popcnt64(_mm256_extract_epi64(r2, 0));
        res += _popcnt64(_mm256_extract_epi64(r2, 1));
        res += _popcnt64(_mm256_extract_epi64(r2, 2));
        res += _popcnt64(_mm256_extract_epi64(r2, 3));
        res += _popcnt64(_mm256_extract_epi64(r3, 0));
        res += _popcnt64(_mm256_extract_epi64(r3, 1));
        res += _popcnt64(_mm256_extract_epi64(r3, 2));
        res += _popcnt64(_mm256_extract_epi64(r3, 3));
    }

    res /= 8;
    res += prefix;

    for (; i < len; ++i) {
        res += str[i] == w;
    }

    return res;
}

typedef unsigned long (*fps_impl_t) (unsigned char*, unsigned long, unsigned char);

fps_impl_t select_fps_simd_impl() {
#ifndef __x86_64__
    return &fps_count_naive;
#else
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    uint32_t ecx1 = 0;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        ecx1 = ecx;
    }

    const bool has_xsave = ecx1 & (1 << 26);
    const bool has_popcnt = ecx1 & (1 << 23);

    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        const bool has_avx2 = has_xsave && (ebx & (1 << 5));
        if (has_avx2 && has_popcnt) {
            return &fps_count_avx2;
        }
    }

    const bool has_sse42 = ecx1 & (1 << 19);
    if (has_sse42 && has_popcnt) {
        return &fps_count_cmpestrm;
    }

    return &fps_count_naive;
#endif
}

unsigned long fps_count(unsigned char *str, unsigned long len, unsigned char w) {
    if (len <= 1024) {
        return fps_count_naive(str, len, w);
    }

    static thread_local fps_impl_t fps_simd_impl = NULL;
    if (!fps_simd_impl) {
        fps_simd_impl = select_fps_simd_impl();
    }

    return (*fps_simd_impl)(str, len, w);
}
