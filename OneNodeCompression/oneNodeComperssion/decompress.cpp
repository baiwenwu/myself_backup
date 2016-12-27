#include"decompress.h"
u32 BitMap[65535];
void CreateBitMap()
{
	u16 R1 = 0,//打头的数字
		R2 = 0,//可解码个数
		R3 = 0,//偏移量
		R4 = 0;//有效解码位数,暂且不用
	u16 mask = -1;
	u32 tmp = -1;
	BitMap[0] = 16ULL << 24;
	for (u32 i = 1; i < (1ULL << 8); i++)
	{
		u32 j = 1ULL << 15;
		u32 num0 = 0;
		while (j)
		{
			if (i&j)
				if (num0)
				{
					BitMap[i] = num0 << 24;
					break;
				}
			num0++;
			j = j >> 1;
		}
	}
	for (u32 i = (1ULL << 8); i < 65536; i++)
	{
		R1 = R2 = R3 = R4 = 0;
		u32 andx = 1ULL << 15;
		u32 num0 = 0;
		while (andx)
		{
			if (i&andx)
			{
				if (!R2)
				{
					R2 = (num0 << 1) + 1;
					R1 = i >> (16 - R2);
				}
				if ((16 - R4)>(num0 << 1))
				{
					R3++;
					R4 += (num0 << 1) + 1;
				}
				andx = andx >> (num0 + 1);
				num0 = 0;
			}
			else
			{
				num0++;
				andx = andx >> 1;
			}
		}
		BitMap[i] += (R4 & 0xff);
		BitMap[i] = (BitMap[i] << 8) + (R3);
		BitMap[i] = (BitMap[i] << 8) + (R2);
		BitMap[i] = (BitMap[i] << 8) + (R1);
	}
}

void printBitMap()
{
	u16 R1 = 0,//打头的数字
		R2 = 0,//可解码个数
		R3 = 0,//偏移量
		R4 = 0;//有效解码位数,暂且不用
	for (u32 i = (1ULL << 8); i < 65536; i++)
	{
		cout << "\t" << bitset<16>(i);
		R1 = BitMap[i] & 0xff;
		R2 = (BitMap[i] >> 8) & 0xff;
		R3 = (BitMap[i] >> 16) & 0xff;
		R4 = (BitMap[i] >> 24) & 0xff;
		cout << "   R4:" << R4;
		cout << "   R3:" << R3;
		cout << "   R2:" << R2;
		cout << "   R1:" << R1;
		cout << endl;
	}
}
void printBitMap_i(u32 i)
{
	cout << "\t" << bitset<16>(i);
	u16 R1 = (BitMap[i]) & 0xff;
	u16 R2 = (BitMap[i] >> 8) & 0xff;
	u16 R3 = (BitMap[i] >> 16) & 0xff;
	u16 R4 = (BitMap[i] >> 24) & 0xff;
	cout << "   R4:" << R4;
	cout << "   R3:" << R3;
	cout << "   R2:" << R2;
	cout << "   R1:" << R1;
	cout << endl;
}

void writeAll1(uchar * src ,u32 index)
{//整块写入
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
void writeDeGammaCode(uchar* src,u32 index,u32 num1)
{
	u32 swords = index >> 3;
	uchar soffset = index & 0x7;
	if (num1 >(8 - soffset))
	{
		index += num1;
		u32 ewords = index >> 3;
		uchar eoffset = index & 0x7;
		src[ewords--] = 0xff << (8 - eoffset);
		while (ewords > swords)
		{
			src[ewords--] = 0xff;
		}
		src[swords] += 0xff>>soffset;//bug 2 is here, and have been verified it
	}
	else
	{
		uchar ch_t = 0xff << (8 - num1);
		src[swords] += ch_t >> soffset;
	}
}
void printBug1(uchar*str,u32 index,u32 len)
{
	u32 offset = index & 0x7;
	u32 num = ((len + offset) >> 3) + 1;
	u32 words = index >> 3;
	for (u32 i = words; i < words + num; i++)
	{
		if (i == 174)
		{
			cout << i << " ";
			int xxxxxxxxxx = 0;
		}
		printBitsOfByte(str[i]);
		cout << " ";
	}	
	cout << endl;
}
u32 decodeGamma(uchar *src,u32 src_index,uchar *des,u32 des_index,bool flag )
{
	u32 decodeBits = des_index+blockSize;//good idea;
	while (1)
	{
		u32 val = getMapBits(src, src_index);
		u32 R1,R2,R3,R4;
		if (val & 0xff00)
		{
			//printBitMap_i(val);
			R1 = BitMap[val] & 0xff;
			R2 = (BitMap[val] >> 8) & 0xff;
			R3 = (BitMap[val] >> 16) & 0xff;
			R4 = (BitMap[val] >> 24) & 0xff;
			R3--;
			//cout << R1 << " ";
			if (flag)
				writeDeGammaCode(des, des_index, R1);
			//printBug1(des, des_index, R1);
			
			flag = !flag;
			des_index += R1;
			src_index += R2;
			if (des_index == decodeBits)
				return src_index;
			while (R3--)
			{
				val = (val << R2) & 0xffff;//bug is here 1 移位后数据超出了界限[已经修改]
				//printBitMap_i(val);
				R1 = BitMap[val] & 0xff;
				//cout << R1 << " ";
				R2 = (BitMap[val] >> 8) & 0xff;
				if (flag)
					writeDeGammaCode(des, des_index, R1);
				//printBug1(des, des_index, R1);
				flag = !flag;
				des_index += R1;
				src_index += R2;
				if (des_index == decodeBits)//bug is here 2 数据到达上界后，src_index没有加上其R4的部分
					return src_index;
			}
			//src_index += R4;
		}
	}
	return 0;
}
void derunLengthHybirdCode(fileStream *node)
{
	//u32 indexs = 0;
	uchar *tmp_src;
	u32 tmp_srcLen = 0;
	u32 decLen = 0;
	u32 errs;
	creatUcharArr(&tmp_src, node->srcNum+36);
	u32 i = 0;
	int block_i = 0;
	while (tmp_srcLen < node->srcLen)
	{
		//cout << "block_i=" << block_i++ << endl;
		switch (node->head->getValue_i(i++))
		{
		case 0:
			break;
		case 1:
			writeAll1(tmp_src,tmp_srcLen);//can use the function [writeDeGammaCode(des, des_index, R1);]
			break;
		case 2://plain
			copyBits(node->cdata, decLen, tmp_src, tmp_srcLen, blockSize);
			decLen += blockSize;break;
		case 3:
			decLen = decodeGamma(node->cdata, decLen, tmp_src, tmp_srcLen, 1);
			 errs= testDecompressionStr(node->src, blockSize, tmp_srcLen, tmp_src);
			if (errs)
			{
				cout << "错误个数：" << errs << endl;
				cout << "源串：" << endl;
				printString(node->src, tmp_srcLen, 32);
				cout << "解压：" << endl;
				printString(tmp_src, tmp_srcLen, 32);
				exit(0);
			}			
			break;
		case 4:
			decLen = decodeGamma(node->cdata, decLen, tmp_src, tmp_srcLen, 0);
			errs = testDecompressionStr(node->src, blockSize, tmp_srcLen, tmp_src);
			if (errs)
			{
				cout << "错误个数：" << errs << endl;
				cout << "源串：" << endl;
				printString(node->src, tmp_srcLen, 32);
				cout << "解压：" << endl;
				printString(tmp_src, tmp_srcLen, 32);
				exit(0);
			}
			break;
		default:
			break;
		}
		tmp_srcLen += blockSize;
	}
	testDecompressionStr(node->src, node->srcNum, 0, tmp_src);
}

u32 testDecompressionStr(uchar * src, u32 blsize, u32 srcIndex, uchar * des)
{
	cout << endl;
	//printString(des, srcIndex, 32);
	u32 erorrNum = 0;
	u32 srcWords = srcIndex >> 3;
	u32 srcNum = srcWords + (blsize >> 3);
	if (srcIndex & 0x7)
	{
		cout << "传入参数错误，请更正！" << endl;
	}
	for (; srcWords < srcNum; srcWords++)
	{
		if (src[srcWords] != des[srcWords])
		{
			cout << "error " << srcWords << "\t" << src[srcWords] << "\t" << des[srcWords] << endl;
			erorrNum++;
		}
	}
	return erorrNum;
}
