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

#pragma once

// Defines
#define CT2_EXT ".ct2"
#define CT2_MAGIC "CTK2"
#define CT2_CAB_A "CA\0\0"
#define CT2_CAB_B "CB\0\0"
#define CT2_DATA "DA"
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))

// Structs
#pragma pack(push, 1)
struct SCh {
	unsigned char ID[2];
	unsigned short size;
};

struct STKCab {
	unsigned char name[6];
	unsigned char numberOfBlocks;
	unsigned char actualBlock;
};

struct STKAddr {
	unsigned short initialAddr;
	unsigned short endAddr;
};
#pragma pack(pop)

struct Tk2kBinary {
	char name[7];
	int  numberOfBlocks;
	int  initialAddr;
	int  endAddr;
	int  size;
	char *data;
};

struct Ct2File {
	int numOfBinaries;
	struct Tk2kBinary *binaries[];
};

// Prototipes
struct Ct2File *readCt2FromBuffer(const char *buffer, 
		const unsigned long size);
struct Ct2File *readCt2FromFile(const char *filename);
struct STKCab *makeCab(char *name, int numberOfBlocks, int actualBlock);
char *makeDataBlock(struct STKCab *dh, char *data, 
		unsigned int len, int *outLen);
int calcCt2BufferSize(unsigned short size);
int createOneCt2Binary(struct Tk2kBinary *binary, char *buffer);
