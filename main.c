#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "impls.h"

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
