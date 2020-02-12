#include "impls.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
	const unsigned char ch = 'w';
	const int totalLen = 500;
	unsigned char str[totalLen];
	memset(str, ch, totalLen);
	for (int start = 0; start < 64; ++start) {
		for (int len = 250; len < totalLen - start; len += 16) {
			const int count_c = fps_count_cmpestrm(str + start, len, ch);
			if (count_c != len) {
				printf("count_cmpestrm failed: %d %d %d\n", start, len, count_c);
			}
			const int count_2 = fps_count_256(str + start, len, ch);
			if (count_2 != len) {
				printf("count_avx2 failed: %d %d %d\n", start, len, count_2);
			}
		}
	}
}
