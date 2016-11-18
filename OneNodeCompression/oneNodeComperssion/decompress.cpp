#include"decompress.h"
void writeAll1(uchar * src ,u32 index)
{//Õû¿éÐ´Èë
	uchar ch_t = 0xff;
	u32 words = index >> 3;
	u32 offset = index & 0x7;
	src[words++] = ch_t >> offset;
	u32 num = blockSize >> 3;
	u32 i = 0;
	for (; i < num; i++)
		src[words++] = 0xff;
	src[words++] = ch_t >> (8 - offset);
}
void derunLengthHybirdCode(fileStream *node)
{
	u32 indexs = 0;
	uchar *tmp_src;
	u32 tmp_srcLen = 0;
	creatUcharArr(&tmp_src, node->srcNum+1);
	u32 i = 0;
	while (indexs < node->srcLen)
	{
		switch (node->head->getValue_i(i++))
		{
		case 0:
			tmp_srcLen += blockSize; break;
		case 1:
			writeAll1(tmp_src,tmp_srcLen);
			tmp_srcLen += blockSize; break;
		default:
			break;
		}
	}
}