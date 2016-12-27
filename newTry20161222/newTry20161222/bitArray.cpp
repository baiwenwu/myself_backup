#include "bitArray.h"
#include<time.h>
u32 blockSize = 256;
bitArray::bitArray(u32 bitsLen, uchar bits)
{
	arr = NULL;
	bitLen = bitsLen;
	setBits = bits;
	mask = (1 << bits) - 1;
	//if (!bitsLen||!bits)
	//{
	//	printf("bits数组输入参数错误!\n");
	//}
	u32 headNum = bitsLen / blockSize;
	arrLen = ((headNum*bits) >> bits) + 2;
	arr = (uchar *)malloc(arrLen*sizeof(uchar));
	memset(arr, 0, arrLen*sizeof(uchar));
	if (!arr)
	{
		printf("calloc head error! \n");
	}
	currIndex = 0;
	blockNum = 0;
}
int bitArray::writeValue(u16 type)
{
	if (!arr)
	{
		printf("write type to headArray error!\n");
		return -1;
	}
	u32 words = currIndex >> setBits;
	u32 offset = currIndex & 7u;
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
	u32 offset = index & 7;
	u16 tmp = arr[words++];
	tmp = tmp << 8;
	tmp += arr[words];
	return (tmp >> (16 - setBits - offset)) & mask;
}
int bitArray::testArr()
{
	if (!arr)
	{
		printf("the arr is NULL ,you can't write or get value !\n");
		return -1;
	}
	return 0;
}
void bitArray_test()
{
	u32 headLen = 0;
	int NUM = 10000;

	u16 randArray[10000] = { 0 };
	bitArray head(NUM * 256, 3);
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
	size += sizeof(uchar)*arrLen;
	return 0;

}
