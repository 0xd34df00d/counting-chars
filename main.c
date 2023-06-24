#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "impls.h"

unsigned long get_utime() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
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
        ptr = &fps_count_avx2;
        break;
    case 'd':
        ptr = &fps_count;
        break;
    }

    const int fd = open(argv[1], O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);

    unsigned char *str = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    const unsigned long t1 = get_utime();

    unsigned long cnt = 0;
    for (int i = 0; i < iters; ++i)
        cnt += (*ptr)(str, sb.st_size, w);

    const unsigned long t2 = get_utime();

    printf("%lu %f\n", cnt / iters, (t2 - t1) / (10 * 1e3));
}
