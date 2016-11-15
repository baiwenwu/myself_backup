#include"decompress.h"
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
			writeAll1();
			tmp_srcLen += blockSize; break;
		default:
			break;
		}
	}
}