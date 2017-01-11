#pragma once
#include"wzip.h"

struct bitArray{
	uchar * arr;
	u32 bitLen;
	u32 currIndex;
	u32 currWords;
	uchar currOffset;
	uchar setBits;
	uchar mask;
	u32 blockNum;
	bitArray(u32 bitsLen, uchar bits, u32 BlockSize);
	int writeValue(u16 type);
	void setCurrValue(u32 index, u32 words, uchar offset);
	int getValue_i(u32 i);
	int getValue_index();
	void destroyBtiArray();
	int getBitsLenofArray();
	int testArr();
	int structSize();
};