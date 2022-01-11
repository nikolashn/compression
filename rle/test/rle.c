// See LICENSE for copyright/license information

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define DECMPBUFSIZE 0x1000
#define CMPBUFSIZE 0x1800

int main(int argc, char* argv[]) {
	if (argc != 4) {
		goto LBL_USAGE_ERR;
	}

	_Bool isCompress = 0;
	if (!strcmp(argv[1], "c")) {
		isCompress = 1;
	}
	else if (!strcmp(argv[1], "d")) {
		isCompress = 0;
	}
	else goto LBL_USAGE_ERR;

	FILE* inFile = fopen(argv[2], "r");
	if (!inFile) {
		fprintf(stderr, "Error: file '%s' not found", argv[2]);
		return -1;
	}

	FILE* outFile = fopen(argv[3], "wb");
	if (!outFile) {
		fprintf(stderr, "Error: could not open file '%s' for writing", argv[3]);
		return -1;
	}

	char decmpBuf[DECMPBUFSIZE] = {0};
	char cmpBuf[CMPBUFSIZE] = {0};
	char prevChar = 0;
	char currChar = 0;

	if (isCompress) {
		unsigned char runs = 1;
		while (fread(decmpBuf, 1, DECMPBUFSIZE, inFile)) {
			int ii = 0;
			int oo = 0;
			currChar = decmpBuf[ii];
			while (1) {
				if (currChar == prevChar && runs < 255) {
					++runs;
				}
				else if (prevChar && runs == 1) { 
					cmpBuf[oo] = prevChar;
					++oo;
				}
				else if (prevChar) {
					cmpBuf[oo] = prevChar;
					cmpBuf[oo+1] = prevChar;
					cmpBuf[oo+2] = runs;
					oo += 3;
					runs = 1;
				}
				if (!currChar) {
					break;
				}
				++ii;
				prevChar = currChar;
				currChar = ii >= DECMPBUFSIZE ? 0 : decmpBuf[ii];
			}
			fwrite(cmpBuf, 1, oo, outFile);
		}
	}
	else {
		_Bool isRunCntNext = 0;
		while (fread(cmpBuf, 1, CMPBUFSIZE, inFile)) {
			int ii = 0;
			int oo = 0;
			while (1) {
				currChar = ii >= CMPBUFSIZE ? 0 : cmpBuf[ii];
				if (isRunCntNext) {
					int runCnt = currChar;
					for (int r = 0; r < runCnt; ++r) {
						decmpBuf[oo+r] = prevChar;
					}
					oo += runCnt;
					prevChar = 0;
					isRunCntNext = 0;
					++ii;
					continue;
				}
				else if (prevChar && currChar == prevChar) {
					isRunCntNext = 1;
				}
				else if (prevChar) {
					decmpBuf[oo] = prevChar;
					++oo;
				}
				++ii;
				prevChar = currChar;
				if (!currChar) {
					break;
				}
			}
			fwrite(decmpBuf, 1, oo, outFile);
		}
	}

	fclose(outFile);
	fclose(inFile);
	return 0;

	LBL_USAGE_ERR:
		fprintf(stderr, "Usage: c/d input output\n"
			"  c - compress\n  d - decompress\n");
		return -1;
}
