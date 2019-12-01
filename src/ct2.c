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
char *makeDataBlock(struct STKCab *dh, 
		unsigned char *data, unsigned int len) {
	int i, t = sizeof(struct STKCab);
	char *out = (char *)malloc(t + len + 1);
	unsigned char cs = 0xFF;

	memcpy(out, dh, t);
	memcpy(out+t, data, len);
	for (i = 0; i < (t+len); i++) {
		cs ^= (unsigned char)(out[i]);
	}
	out[t+len] = (unsigned char)cs;
	return out;
}

/*****************************************************************************/
int calcCt2BufferSize(unsigned short size) {
	int m, n, tamBuf;

	/* Calculate size of buffer */
	n = size / 256;
	m = size % 256;
	tamBuf = 4;
	tamBuf += 4 + 4 + sizeof(struct STKCab) + sizeof(struct STKEnd) + 1;
	tamBuf += n * (4 + 4 + sizeof(struct STKCab) + 256 + 1);
	if (m)
		tamBuf += 4 + 4 + sizeof(struct STKCab) + m + 1;
	return tamBuf;
}

/*****************************************************************************/
int makeCt2File(char *name, unsigned char numberOfBlocks, 
		unsigned short initialAddr, unsigned short endAddr,
		char *data, unsigned short size, char *buffer) {
	int t, da;
	char *p, *pc, *pd, *l;
	unsigned char cs;
	unsigned short tt;
	struct STKCab *tkcab;
	struct STKEnd tkend;

	p = buffer;

	// Header A
	memcpy(p, CT2_CAB_A, 4);
	p += 4;
	// Block 0
	memcpy(p, CT2_CAB_B, 4);
	p += 4;
	memcpy(p, CT2_DATA, 2);
	p += 2;
	tt = sizeof(struct STKCab) + sizeof(struct STKEnd) + 1;
	memcpy(p, &tt, 2);
	p += 2;
	tkcab = makeCab(name, numberOfBlocks, 0);
	tkend.initialAddr = initialAddr;
	tkend.endAddr = endAddr;
	pc = p;
	memcpy(p, &tkcab, sizeof(struct STKCab));
	p += sizeof(struct STKCab);
	memcpy(p, &tkend, sizeof(struct STKEnd));
	p += sizeof(struct STKEnd);
	// calculate checksum
	cs = 0xFF;
	for (l = pc; l < p; l++) {
		cs ^= *l;
	}
	memcpy(p, &cs, 1);
	p += 1;
	da = size;
	pd = data;
	while (da > 0) {
		if (da > 256)
			t = 256;
		else
			t = da;
		memcpy(p, CT2_CAB_B, 4);
		p += 4;
		memcpy(p, CT2_DATA, 2);
		p += 2;
		tt = t + sizeof(struct STKCab) + 1;
		memcpy(p, &tt, 2);
		p += 2;
		tkcab->actualBlock++;
		pc = p;
		memcpy(p, &tkcab, sizeof(struct STKCab));
		p += sizeof(struct STKCab);
		memcpy(p, pd, t);
		p += t;
		// calculate checksum
		cs = 0xFF;
		for (l = pc; l < p; l++) {
			cs ^= *l;
		}
		memcpy(p, &cs, 1);
		p += 1;
		pd += t;
		da -= t;
	}
	free(tkcab);
	return 1;
}

