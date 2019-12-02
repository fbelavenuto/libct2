/* libct2 - Library to work with CT2 files
 *
 * Copyright (C) 2014-2020  Fabio Belavenuto
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ct2.h"

// Functions

/*****************************************************************************/
struct Ct2File *readCt2FromBuffer(const char *buffer, const unsigned long size) {
	struct Ct2File *result = NULL;
	struct SCh ch;
	struct STKCab th;
	struct STKAddr ta;
	unsigned long posBuf = 4;
	struct Tk2kBinary *binary = NULL;
	int i, numOfBinaries = 0, posData = 0;

	// Check MAGIC
	if (memcmp(buffer, CT2_MAGIC, 4) != 0) {
		return NULL;
	}

	while (posBuf < size) {
		memcpy(&ch, &buffer[posBuf], 4);
		posBuf += 4;
		// If is DATA chunk
		if (memcmp(ch.ID, CT2_DATA, 2) == 0) {
			memcpy(&th, &buffer[posBuf], 8);
			posBuf += 8;
			// If is first block
			if (th.actualBlock == 0) {
				if (NULL == result) {
					result = (struct Ct2File *)malloc(sizeof(struct Ct2File) + sizeof(struct Tk2kBinary *));
					if (NULL == result) {
						return NULL;
					}
					memset(result, 0, sizeof(struct Ct2File) + sizeof(struct Tk2kBinary *));
				} else {
					size_t newSize = sizeof(struct Ct2File) + sizeof(struct Tk2kBinary *) * (numOfBinaries+1);
					result = (struct Ct2File *)realloc(result, newSize);
					if (NULL == result) {
						return NULL;
					}
				}
				memcpy(&ta, &buffer[posBuf], 4);
				posBuf += 4;
				binary = (struct Tk2kBinary *)malloc(sizeof(struct Tk2kBinary));
				result->binaries[numOfBinaries++] = binary;
				memset(binary, 0, sizeof(struct Tk2kBinary));
				// copy informations
				for (i = 0; i < 6; i++) {
					binary->name[i] = th.name[i] & 0x7F;
				}
				binary->numberOfBlocks = th.numberOfBlocks;
				binary->initialAddr = ta.initialAddr;
				binary->endAddr = ta.endAddr;
				binary->size = ta.endAddr - ta.initialAddr + 1;
				// alloc data
				binary->data = (char *)malloc(binary->size);
				if (NULL == binary->data) {
					return NULL;
				}
				posData = 0;
				posBuf += 1; // skip checksum
			} else {
				int lenData = ch.size - 9;
				memcpy(&binary->data[posData], &buffer[posBuf], lenData);
				posData += lenData;
				posBuf += ch.size - 8; // skip checksum
			}
		}
	}
	result->numOfBinaries = numOfBinaries;
	return result;
}

/*****************************************************************************/
struct Ct2File *readCt2FromFile(const char *filename) {
	struct Ct2File *result = NULL;
	char *buffer = NULL;
	unsigned long fileSize;

	FILE *fileBin = fopen(filename, "rb");
	if (!fileBin)
		return NULL;
	fseek(fileBin, 0, SEEK_END);
	fileSize = (unsigned long)(ftell(fileBin));
	fseek(fileBin, 0, SEEK_SET);
	buffer = (char *)malloc(fileSize);
	fread(buffer, 1, fileSize, fileBin);
	fclose(fileBin);
	result = readCt2FromBuffer(buffer, fileSize);
	free(buffer);
	return result;
}

/*****************************************************************************/
struct STKCab *makeCab(char *name, int numberOfBlocks, int actualBlock) {
	unsigned int i;
	struct STKCab *dh = 
		(struct STKCab *)malloc(sizeof(struct STKCab));
	memset(dh, 0, sizeof(struct STKCab));
	memset(&dh->name, 0xA0, 6);
	for (i=0; i < MIN(strlen(name), 6); i++) {
		dh->name[i] = name[i] | 0x80;
	}
	dh->numberOfBlocks = numberOfBlocks;
	dh->actualBlock = actualBlock;
	return dh;
}

/*****************************************************************************/
char *makeDataBlock(struct STKCab *dh, char *data, 
		unsigned int len, int *outLen) {
	int i, t;
	char *out = NULL;
	unsigned char cs = 0xFF;

	t = sizeof(struct STKCab);
	*outLen = t + len + 1;
	out = (char *)malloc(*outLen);
	memcpy(out, dh, t);
	memcpy(out+t, data, len);
	for (i = 0; i < (t+len); i++) {
		cs ^= (unsigned char)(out[i]);
	}
	out[t+len] = (unsigned char)cs;
	return out;
}

/*****************************************************************************/
int calcCt2BufferSize(const unsigned short size) {
	int m, n, sizeOfBuffer;

	/* Calculate size of buffer */
	n = size / 256;
	m = size % 256;
	sizeOfBuffer = 4;
	sizeOfBuffer += 4 + 4 + sizeof(struct STKCab) + sizeof(struct STKAddr) + 1;
	sizeOfBuffer += n * (4 + 4 + sizeof(struct STKCab) + 256 + 1);
	if (m)
		sizeOfBuffer += 4 + 4 + sizeof(struct STKCab) + m + 1;
	return sizeOfBuffer;
}

/*****************************************************************************/
int createOneCt2Binary(struct Tk2kBinary *binary, const char *buffer) {
	int chunkSize, outSize, sizeOfBinary;
	char *p, *po, *pd;
	unsigned short tt;
	struct STKCab *tkcab;
	struct STKAddr tkend;

	p = (char *)buffer;

	// Header A
	memcpy(p, CT2_CAB_A, 4);
	p += 4;
	// Block 0
	memcpy(p, CT2_CAB_B, 4);
	p += 4;
	memcpy(p, CT2_DATA, 2);
	p += 2;
	tt = sizeof(struct STKCab) + sizeof(struct STKAddr) + 1;
	memcpy(p, &tt, 2);
	p += 2;
	tkcab = makeCab(binary->name, binary->numberOfBlocks, 0);
	tkend.initialAddr = binary->initialAddr;
	tkend.endAddr = binary->endAddr;
	po = makeDataBlock(tkcab, (char *)&tkend, sizeof(struct STKAddr), &outSize);
	memcpy(p, po, outSize);
	p += outSize;
	sizeOfBinary = binary->size;
	pd = binary->data;
	while (sizeOfBinary > 0) {
		if (sizeOfBinary > 256) {
			chunkSize = 256;
		} else {
			chunkSize = sizeOfBinary;
		}
		memcpy(p, CT2_CAB_B, 4);
		p += 4;
		memcpy(p, CT2_DATA, 2);
		p += 2;
		tt = chunkSize + sizeof(struct STKCab) + 1;
		memcpy(p, &tt, 2);
		p += 2;
		tkcab->actualBlock++;
		po = makeDataBlock(tkcab, pd, chunkSize, &outSize);
		memcpy(p, po, outSize);
		p += outSize;
		pd += chunkSize;
		sizeOfBinary -= chunkSize;
	}
	free(tkcab);
	return 1;
}
