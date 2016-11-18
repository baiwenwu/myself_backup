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
inline u32 getMapBits(uchar *src, u32 index)
{
	u32 words = index >> 3;
	uchar offset = index & 0x7;
	u32 tmp = src[words++];
	tmp = (tmp<<8)+src[words++];
	tmp = (tmp << 8) + src[words++];
	tmp = tmp >> (8 - offset);
	return tmp & 0xffff;
}
void writeDeGammaCode(uchar src,u32 index,u32 num1)
{

}
void decodeGamma(uchar *src,u32 src_index,uchar *des,u32 des_index,uchar flag )
{
	u32 decodeBits = 0;
	while (1)
	{
		u32 val = getMapBits(src, src_index);
		if (val & 0xff00)
		{
			u32 R1 = val & 0xff;
			u32 R2 = (val>>8) & 0xff;
			u32 R3 = (val >> 16) & 0xff;
			u32 R4 = (val >> 24) & 0xff;
			R3--;
			while (R3)
			{

			}
		}
	}
}
void derunLengthHybirdCode(fileStream *node)
{
	u32 indexs = 0;
	uchar *tmp_src;
	u32 tmp_srcLen = 0;
	u32 decLen = 0;
	creatUcharArr(&tmp_src, node->srcNum+1);
	u32 i = 0;
	while (indexs < node->srcLen)
	{
		switch (node->head->getValue_i(i++))
		{
		case 0:
			break;
		case 1:
			writeAll1(tmp_src,tmp_srcLen);
			break;
		case 2://plain
			copyBits(node->cdata, decLen, tmp_src, tmp_srcLen, blockSize);
			decLen += blockSize;break;
		case 4:
		case 5:
		default:
			break;
		}
	}
}