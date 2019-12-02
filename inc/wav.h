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
#define DURSILENP	300
#define DURSILENS	500
#define DURCABA		1000
//#define MINCABA		500
//#define MAXCABA		3000
#define TK2000_BIT0 2000
#define TK2000_BIT1 1000
#define	TK2000_CABB 720
#ifndef WAVE_FORMAT_PCM
# define WAVE_FORMAT_PCM 0x0001 /* Microsoft Corporation */
#endif

// Structs
#pragma pack(push, 1)
typedef struct SWaveCab {
	unsigned char  groupID[4];		// RIFF
	unsigned int   groupLength;
	unsigned char  typeID[4];		// WAVE
	unsigned char  formatID[4];		// fmt
	unsigned int   formatLength;
	unsigned short wFormatTag;
	unsigned short numChannels;
	unsigned int   samplesPerSec;
	unsigned int   bytesPerSec;
	unsigned short nBlockAlign;
	unsigned short bitsPerSample;
	unsigned char  dataID[4];
	unsigned int   dataLength;
} TWaveCab, *PTWaveCab;
#pragma pack(pop)

// Enums
enum WaveFormat {
	WF_SQUARE = 0,
	WF_SINE
};

// Prototipes
void wavConfig(enum WaveFormat to, unsigned int ta,
		unsigned int bi, double vol, int inv);
int createWaveFile(char *filename);
int playSilence(int durationMs);
int playTone(int frequency, int durationMs, double dutyCycle);
int finishWaveFile();
int tk2kPlayByte(unsigned char c);
int tk2kPlayBuffer(unsigned char *buffer, int len);
int tk2kPlayBin(char *data, int len, char *name, int initialAddr);
