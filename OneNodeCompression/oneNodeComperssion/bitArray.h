#pragma once
#include "baseLib.h"
#include "global.h"

struct bitArray{
	uchar * arr;
	u32 bitLen;
	u32 currIndex;
	u32 arrLen;
	uchar setBits;
	uchar mask;
	u32 blockNum;
	bitArray(u32 bitsLen,uchar bits);
	int writeValue(u16 type);
	int getValue_i(u32 i);
	int getValue_index(u32 index);
	int testArr();
	int structSize();
};
void bitArray_test();