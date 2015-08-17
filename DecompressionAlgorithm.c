#include <stdio.h>
#include <stdlib.h>

typedef struct {
	unsigned int length;
	unsigned int bits;
	signed int value;
} VLC;


VLC vlcTable[] = {
	{12,    0,     15},
	{ 2,    0,      0},
	{ 3,    2,     -1},
	{ 3,    3,     -3},
	{ 3,    4,     -7},
	{ 3,    5,    -15},
	{ 3,    6,    -31},
	{ 4,   14,    -63},
	{ 5,   30,   -127},
	{ 6,   62,   -255},
	{ 7,  126,   -511},
	{ 8,  254,  -1023},
	{ 9,  510,  -2047},
	{10, 1022,  -4095},
	{11, 2046,  -8191},
	{12, 4094, -16383}
};

unsigned char Bitstream1[] = {
	0x0A, 0xFE, 0x81, 0xE8, 0x1F, 0xE8, 0x01, 0xD0, 0x7C, 0x12, 0x00, 0x02
};

unsigned char Bitstream2[] = {
	0x57, 0xd3, 0xf4, 0x5f, 0x49, 0xff, 0x46, 0x5d, 0xdf, 0x8a, 0xc0, 0x00
};

unsigned char Bitstream3[] = {
	0x83, 0x47, 0xec, 0x4f, 0xf7, 0xe7, 0xff, 0xb9, 0xbf, 0xf7, 0x47, 0xb9, 0xff, 0xd7, 0xd0, 0x00
};

unsigned char Bitstream4[] = {
	0x37, 0x3a, 0x3d, 0xef, 0xfb, 0x9b, 0xf7, 0xbb, 0x9f, 0xfd, 0x7d, 0x00
};

unsigned char Bitstream[] = {
	0x10, 0x57, 0x0d, 0x00
};

typedef struct
{
	unsigned char *bitstream;
	unsigned char bitsUsed;
	unsigned int  bytesUsed;
	unsigned int  validBytes;
	unsigned int  bitsRead;
	unsigned int  vlc;
} decState;

unsigned int ReadBits(decState*, int);
unsigned int GetBits(decState*, int);
signed int vlcDecode(decState*, unsigned int, VLC*);

int main(void)
{
	decState state;
	unsigned int vlc;
	signed int decodedValue, bsi, index, bitsDecoded;

	/* Initializing the decoder state structure */
	state.bitstream  = Bitstream;
	state.bitsUsed   = 0;
	state.bytesUsed	 = 0;
	state.bitsRead   = 0;

	state.validBytes = sizeof(Bitstream);

	while(1) {
		if(state.validBytes == state.bytesUsed) break;
		vlc = ReadBits(&state, vlcTable[0].length);
		if (!state.bitsRead) break;
		printf ("VLC = %x\n", vlc);
		index = vlcDecode(&state, vlc, vlcTable);
		if (index == -1) {
			printf ("Error: Invalid VLC...\n");
			break;
		}
		decodedValue = vlcTable[index].value;
		bitsDecoded = state.bitsRead;
		vlc = GetBits(&state, state.bitsRead);
		if ((index - 1)) {
			unsigned int ai;

			ai = GetBits(&state, index - 1);
			bitsDecoded += index - 1;
			if (ai < (unsigned int)((1 << index) >> 2))
				bsi = decodedValue + ai;
			else
				bsi = ai;
		}
		else {
			bsi = decodedValue;
		}

		printf ("Bits decoded using VLC = %d\n", bitsDecoded);
		printf ("GetBits vlc = %x\n", vlc);
		printf ("Decoded Value = %d\n", decodedValue);
		printf ("Final Decoded Value = %d\n", bsi);
		printf ("Bytes Used = %d\n", state.bytesUsed);
		printf ("Bits  Used = %d\n", state.bitsUsed);
		printf ("---------------------------------------\n");
		//getchar();
	}
			
	return 0;
}

signed int vlcDecode(decState *state, unsigned int vlc, VLC vlcTable[])
{
	signed int vlcIndex;
	for (vlcIndex = 1; vlcIndex <= vlcTable[0].value; vlcIndex++) {
		if ((vlc >> (vlcTable[0].length - vlcTable[vlcIndex].length)) == vlcTable[vlcIndex].bits) {
			state->bitsRead = vlcTable[vlcIndex].length;
			return vlcIndex;
		}
	}
	return -1;
}

unsigned int ReadBits(decState *state, int len)
{
	unsigned char *stream;
	unsigned int vlc = 0;
	unsigned int bitsRemaining;

	stream = state->bitstream + state->bytesUsed;
	if ((state->bytesUsed + ((state->bitsUsed + len + 7) >> 3)) <= state->validBytes) {
		vlc = *stream;
		vlc = (vlc << (24 + state->bitsUsed)) >> (24 + state->bitsUsed);
		state->bitsRead = len;
		if (len < (8 - state->bitsUsed)) {
			vlc = vlc >> (8 - state->bitsUsed - len);
			// state->bitsUsed += len;
		}
		else {
			// state->bytesUsed++;
			bitsRemaining = len - (8 - state->bitsUsed);
			while(bitsRemaining != 0) {
				if (bitsRemaining < 8) {
					unsigned char vlcIntermediate = 0;
					vlcIntermediate = *++stream;
					vlcIntermediate >>= (8 - bitsRemaining);
					vlc = (vlc << bitsRemaining) | vlcIntermediate;
					bitsRemaining = 0;
					// state->bitsUsed = bitsRemaining;
				}
				else {
					vlc = (vlc << 8) | *++stream;
					bitsRemaining -= 8;
					// state->bytesUsed++;
					// state->bitsUsed = 0;
				}
			}
		}
	}
	else {
		bitsRemaining = ((state->validBytes - state->bytesUsed) << 3) - state->bitsUsed;
		state->bitsRead = bitsRemaining;
		vlc = stream[0];
		vlc = (vlc << (24 + state->bitsUsed)) >> (24 + state->bitsUsed);
		bitsRemaining -= (8 - state->bitsUsed);
		// state->bytesUsed++;
		while(bitsRemaining != 0) {
			vlc = (vlc << 8) | *++stream;
			bitsRemaining -= 8;
			// state->bytesUsed++;
		}
		// state->bitsUsed = 0;
		// state->bytesUsed = state->validBytes;
	}
	state->vlc = vlc;
	return vlc;
}

unsigned int GetBits(decState *state, int len)
{
	
	unsigned char *stream;
	unsigned int vlc = 0;
	unsigned int bitsRemaining;

	stream = state->bitstream + state->bytesUsed;
	if ((state->bytesUsed + ((state->bitsUsed + len + 7) >> 3)) <= state->validBytes) {
		vlc = *stream;
		vlc = (vlc << (24 + state->bitsUsed)) >> (24 + state->bitsUsed);
		state->bitsRead = len;
		if (len < (8 - state->bitsUsed)) {
			vlc = vlc >> (8 - state->bitsUsed - len);
			state->bitsUsed += len;
		}
		else {
			state->bytesUsed++;
			bitsRemaining = len - (8 - state->bitsUsed);
			state->bitsUsed = 0;
			while(bitsRemaining != 0) {
				if (bitsRemaining < 8) {
					unsigned char vlcIntermediate = 0;
					vlcIntermediate = *++stream;
					vlcIntermediate >>= (8 - bitsRemaining);
					vlc = (vlc << bitsRemaining) | vlcIntermediate;
					state->bitsUsed = bitsRemaining;
					bitsRemaining = 0;
				}
				else {
					vlc = (vlc << 8) | *++stream;
					bitsRemaining -= 8;
					state->bytesUsed++;
					state->bitsUsed = 0;
				}
			}
		}
	}
	else {
		bitsRemaining = ((state->validBytes - state->bytesUsed) << 3) - state->bitsUsed;
		state->bitsRead = bitsRemaining;
		vlc = stream[0];
		vlc = (vlc << (24 + state->bitsUsed)) >> (24 + state->bitsUsed);
		bitsRemaining -= (8 - state->bitsUsed);
		state->bytesUsed++;
		while(bitsRemaining != 0) {
			vlc = (vlc << 8) | *++stream;
			bitsRemaining -= 8;
			state->bytesUsed++;
		}
		state->bitsUsed = 0;
	}
	state->vlc = vlc;
	return vlc;
}
