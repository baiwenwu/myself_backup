#include "bitArray.h"
#include<time.h>
bitArray::bitArray(u32 bitsLen, uchar bits,u32 BlockSize)
{
	arr = NULL;
	bitLen = bitsLen;
	setBits = bits;
	mask = (1 << bits) - 1;
	if (bitsLen != 0 && bits != 0 && BlockSize != 0)
	{
		u32 headNum = bitsLen / BlockSize;
		u32 arrLen = headNum >> 1;
		arr = new uchar[arrLen];
		if (!arr)
		{
			cout << "new uchar[arrLen] is error! " << endl;
		}
		memset(arr, 0, arrLen*sizeof(uchar));
	}
	currIndex = 0;
	blockNum = 0;
}
int bitArray::writeValue(u16 type)
{
	u32 words = currIndex >> setBits;
	u32 offset = currIndex & 0x7;
	u16 tmp = arr[words];
	tmp = tmp << 8;
	tmp = (tmp >> (16 - setBits - offset)) + type;
	tmp = tmp << (16 - setBits - offset);
	arr[words++] = (tmp >> 8) & 0xff;
	arr[words] = tmp & 0xff;
	currIndex += setBits;
	blockNum++;
	return 0;
}
int bitArray::getValue_i(u32 i)
{
	u32 index = i*setBits;
	u32 words = index >> setBits;
	u32 offset = index & 0x7;
	u16 tmp = arr[words++];
	tmp = tmp << 8;
	tmp += arr[words];
	return (tmp >> (16 - setBits - offset)) & mask;
}
int bitArray::getValue_index()
{
	u32 words = currIndex >> setBits;
	u32 offset = currIndex & 0x7;
	u16 tmp = arr[words++];
	tmp = tmp << 8;
	tmp += arr[words];
	currIndex += setBits;
	return (tmp >> (16 - setBits - offset)) & mask;
}
void bitArray::setCurrValue(u32 index,u32 words,uchar offset)
{
	currIndex = index;
	currWords = words;
	currOffset = offset;
}
int bitArray::getBitsLenofArray()
{
	return blockNum*setBits;
}
void bitArray::destroyBtiArray()
{
	delete[] arr;
	arr = NULL;
}
void bitArray_test()
{
	u32 headLen = 0;
	int NUM = 10000;

	u16 randArray[10000] = { 0 };
	bitArray head(NUM * 256, 3,256);
	int i = 0;
	srand(time(0));
	for (; i < NUM; i++)
	{
		randArray[i] = rand() % 8;
		u16 t = randArray[i];
		head.writeValue(randArray[i]);
	}
	//test
	for (i = 0; i<NUM; i++)
	{
		u16 type_t = head.getValue_i(i);
		if (randArray[i] != type_t)
		{
			printf("%d error!\n", i);
		}
	}
	printf("the test finished!\n");
}
int bitArray:: structSize()
{
	int size = 0;
	size += sizeof(u32) * 4;
	int nbyte = ((blockNum*setBits) >> 3) + (((blockNum*setBits) & 0x7) ? 1 : 0);
	size += nbyte;
	return 0;

}
