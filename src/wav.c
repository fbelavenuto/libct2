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
#include <unistd.h>
#include <math.h>
#include "wav.h"
#include "ct2.h"


// Variables
static enum WaveFormat waveType  = WF_SINE;
static unsigned int  sampleRate  = 44100;
static unsigned int  bits        = 8;
static double        volume      = 1.0;
static int           waveInv     = 0;
static unsigned int  dataSize    = 0;
static FILE          *fileWav    = NULL;
static TWaveCab      waveCab;

// Public functions

/*****************************************************************************/
void wavConfig(enum WaveFormat to, unsigned int ta, unsigned int bi,
		double vol, int inv) {
	waveType   = to;
	sampleRate = ta;
	bits       = bi;
	volume     = vol;
	waveInv    = inv;
}

/*****************************************************************************/
int createWaveFile(char *filename) {
	size_t s = 0;

	if (!(fileWav = fopen(filename, "wb"))) {
		return 1;
	}
	memset(&waveCab, 0, sizeof(TWaveCab));

	memcpy((char *)waveCab.groupID,  "RIFF", 4);
	waveCab.groupLength    = 0;					// Fill after
	memcpy((char *)waveCab.typeID,   "WAVE", 4);
	memcpy((char *)waveCab.formatID, "fmt ", 4);
	waveCab.formatLength   = 16;
	waveCab.wFormatTag     = WAVE_FORMAT_PCM;
	waveCab.numChannels    = 1;
	waveCab.samplesPerSec  = sampleRate;
	waveCab.bytesPerSec    = sampleRate * 1 * (bits / 8);
	waveCab.nBlockAlign    = 1 * (bits / 8);
	waveCab.bitsPerSample  = bits;
	memcpy((char *)waveCab.dataID,   "data", 4);
	waveCab.dataLength     = 0;					// Fill after
	s = fwrite(&waveCab, 1, sizeof(TWaveCab), fileWav);
	if (s != sizeof(TWaveCab)) {
		return 1;
	}
	dataSize = 0;
	return 0;
}

/*****************************************************************************/
int playSilence(int durationMs) {
	int    total;
	char   *buffer, v;
	size_t t, s = 0;

	if (!fileWav)
		return 1;

	total = (sampleRate * durationMs) / 1000;
	t = bits / 8;
	buffer = (char *)malloc(total * t + t);
	v = (bits == 8) ? 128 : 0;
	memset(buffer, v, total * t);
	s = fwrite(buffer, t, total, fileWav);
	free(buffer);
	if (s != (size_t)total)
		return 1;
	dataSize += total * t;
	return 0;
}

/*****************************************************************************/
int playTone(int frequency, int durationMs, double dutyCycle) {
	short       amp;
	char        *buffer;
	short		*buffer2;
	long double ang, amps, sincos;
	int         t, c, i;
	int         total, cicloT, ciclo1_2;
	size_t      s = 0;

	if (!fileWav)
		return 1;

	const long double PI = acos((long double) -1);

	cicloT = sampleRate / frequency;
	ciclo1_2 = cicloT * dutyCycle;
	total = cicloT * durationMs;
	t = bits / 8;
	buffer = (char *)malloc(total * t + t);
	buffer2 = (short *)buffer;

	if (bits == 8) {
		amp = 127 * volume;
	} else {
		amp = 32767 * volume;
	}

	switch (waveType) {

	case WF_SQUARE:
		ang = 0;
		break;

	case WF_SINE:
		ang = (2 * PI) / cicloT;
		break;
	}

	c = 0;
	while (c < total) {
		switch (waveType) {

		case WF_SQUARE:
			for (i = 0; i < ciclo1_2; i++) {
				if (bits == 8) {
					buffer[c++] = (waveInv) ? 128-amp : 128+amp;
				} else {
					buffer2[c++] = (waveInv) ? -amp : amp;
				}
			}
			for (     ; i < cicloT; i++) {
				if (bits == 8) {
					buffer[c++] = (waveInv) ? 128+amp : 128-amp;
				} else {
					buffer2[c++] = (waveInv) ? amp : -amp;
				}
			}
			break;

		case WF_SINE:
			for (i = 0; i < cicloT; i++) {
				amps = (double)amp;
				sincos = (waveInv) ? -sin(ang * (double)i) : sin(ang * (double)i);
				if (bits == 8) {
					buffer[c++] = (char)(amps * sincos + 128.0);
				} else {
					buffer2[c++] = (short)(amps * sincos);
				}
			}
			break;
		}
	}
	s = fwrite(buffer, t, c, fileWav);
	free(buffer);
	if (s != (size_t)c)
		return 1;
	dataSize += c * t;
	return 0;
}

/*****************************************************************************/
int finishWaveFile() {
	size_t s = 0;
	// Fornecer dados faltantes do cabeçalho
	waveCab.dataLength = dataSize;
	waveCab.groupLength = dataSize + sizeof(TWaveCab) - 8;
	if (fseek(fileWav, 0, SEEK_SET)) {
		return 1;
	}
	s = fwrite(&waveCab, 1, sizeof(TWaveCab), fileWav);
	fclose(fileWav);
	if (s != sizeof(TWaveCab)) {
		return 1;
	}
	return 0;
}

/*****************************************************************************/
int tk2kPlayByte(unsigned char c) {
	int r = 0;
	unsigned char mask;

	for(mask = 0x80; mask; mask >>= 1) {
		if (c & mask)
			r |= playTone(TK2000_BIT1, 1, 0.5);
		else
			r |= playTone(TK2000_BIT0, 1, 0.5);
	}
	return r;
}

/*****************************************************************************/
int tk2kPlayBuffer(unsigned char *buffer, int len) {
	int r = 0, i;

	r |= playTone(TK2000_CABB, 30, 0.5);		// Header B
	r |= playTone(TK2000_BIT0, 1, 0.5);			// Sync
	for (i = 0; i < len; i++) {
		unsigned char c = buffer[i];
		r |= tk2kPlayByte(c);
	}
	return r;
}

/*****************************************************************************/
int tk2kPlayBin(char *data, int len, char *name, int initialAddr) {
	struct STKCab *dh;
	struct STKAddr de;
	char *buffer = NULL;
	int outSize;
	int  r = 0, tb, ba;

	if (len < 1)
		return 1;
	tb = ((len-1) / 256)+1;					// Calculate how many blocks are need
	dh = makeCab(name, tb, 0);				// Create first block
	de.initialAddr = initialAddr;
	de.endAddr = initialAddr + len - 1;
	buffer = makeDataBlock(dh, (char *)&de, sizeof(struct STKAddr), &outSize);
	r |= playSilence(100);
	r |= playTone(TK2000_BIT1, 1000, 0.5);		// Piloto
	r |= tk2kPlayBuffer((unsigned char *)buffer, outSize);
	free(buffer);
	for (ba = 1; ba <= tb; ba++) {
		dh->actualBlock = ba;
		char *p = data + (ba-1) * 256;
		int ts = MIN(256, len);
		len -= 256;
		buffer = makeDataBlock(dh, p, ts, &outSize);
		r |= tk2kPlayBuffer((unsigned char *)buffer, outSize);
		free(buffer);
	}
	free(dh);
	r |= playTone(100, 2, 0.5);					// Final
	r |= playSilence(200);
	return r;
}
