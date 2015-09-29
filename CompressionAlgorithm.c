#include <stdio.h>

typedef struct {
	short value;
	short bits;
} HuffmanVLCTable;

struct {
	unsigned long value;
	unsigned long bits;
} output;

typedef struct {
	unsigned int bsi;
	unsigned int bits;
} HuffEncoded;

HuffmanVLCTable Huffman_VLC_Table[] = {
	{0, 2},			//00
	{2, 3},			//010
	{3, 3},			//011
	{4, 3},			//100
	{5, 3},			//101
	{6, 3},			//110
	{14, 4},		//1110
	{30, 5},		//11110
	{62, 6},		//111110
	{126, 7},		//1111110
	{254, 8},		//11111110
	{510, 9},		//111111110
	{1022, 10},		//1111111110
	{2046, 11},		//11111111110
	{4094, 12}		//111111111110
};

void HuffmanEncode(short, HuffEncoded*);
void fillBits(FILE *fout, HuffEncoded *out);
FILE *fout;

int main(void)
{
	HuffEncoded HuffValue;

	if ((fout = fopen("Encoded.txt", "wb")) == NULL) {
		printf ("Error: Opening output file...\n");
		return -1;
	}

	output.value = 0;
	output.bits  = 0;

	HuffmanEncode(0, &HuffValue);
	fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");
	HuffmanEncode(-1, &HuffValue);
	fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");
	HuffmanEncode(0, &HuffValue);
	fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");
	HuffmanEncode(1, &HuffValue);
	fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");
	HuffmanEncode(2, &HuffValue);
	fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");

	HuffmanEncode(0, &HuffValue);
	fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");
	HuffmanEncode(-2, &HuffValue);
	fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");
	// HuffmanEncode(6096, &HuffValue);
	// fillBits(fout, &HuffValue);
	printf ("Number of bits = %d\n", output.bits);
	printf ("Value = %x\n", output.value);
	printf ("---------------------------------------\n");

	fclose(fout);
	return 0;
}

void HuffmanEncode(short di, HuffEncoded *HuffValue)
{
	short ni = 0, si, copydi, tempdi;
	unsigned int bsi = 0;
	
	HuffValue->bsi = 0;
	HuffValue->bits = 0;

	//	Calculation of log2(|di|)
	tempdi = di;
	if (di) {
		if (di < 0) {
			di = -di;
		}
		copydi = di;
		while (copydi != 1) {
			++ni;
			copydi >>= 1;
		}
		// if (di - (1 << ni))
		++ni;
	}
	di = tempdi;

	//	Extract si from Table
	si = Huffman_VLC_Table[ni].value;

	//	Build bsi
	if (!ni) {
		bsi = si;
	}
	else {
		bsi = si;
		if (di > 0) {
			bsi = (bsi << ni) | di;
		}
		else {
			bsi = (bsi << ni) | ((di - 1) & ((1 << ni) - 1));
		}
	}

	HuffValue->bsi	 = bsi;
	HuffValue->bits  = Huffman_VLC_Table[ni].bits + ni;

	printf ("BSI Value = %x\n", bsi);
	printf ("HuffValue->Bits = %d\n", HuffValue->bits);

	return;
}

void fillBits(FILE *fout, HuffEncoded *out)
{
	if ((output.bits + out->bits) <= 32) {
		output.value |= out->bsi << (32 - out->bits - output.bits);
		output.bits  += out->bits;
		out->bsi = 0;
		out->bits = 0;
		if (output.bits == 32) {
			fwrite(&output.value, sizeof(unsigned int), 1, fout);
		}
	}
	else {
		unsigned long no_of_bits, mask = 0;
		no_of_bits = 32 - output.bits;
		mask = ((1 << no_of_bits) - 1) << (out->bits - no_of_bits);
		output.value |= (out->bsi & mask) >> (out->bits - no_of_bits);
		fwrite(&output.value, sizeof(unsigned int), 1, fout);
		printf ("*********** 32-Bit Dumped Output ******************\n");
		fprintf (stdout, "Output Value = %08x\n", output.value);
		printf ("*********** 32-Bit Dumped Output ******************\n");
		out->bsi &= ~mask;
		out->bits -= no_of_bits;
		output.value = 0;
		output.bits = 0;
		output.value |= out->bsi << (32 - out->bits - output.bits);
		output.bits  += out->bits;
		out->bsi = 0;
		out->bits = 0;
	}

	return;
}
