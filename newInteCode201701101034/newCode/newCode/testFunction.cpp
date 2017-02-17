#include"testFunction.h"
#include"oneplus.h"
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
		R4 = 16 - R4;
		BitMap[i] += (R4 & 0xff);
		BitMap[i] = (BitMap[i] << 8) + (R3);
		BitMap[i] = (BitMap[i] << 8) + (R2);
		BitMap[i] = (BitMap[i] << 8) + (R1);
	}
}

void writePlusOne(u32 num, uchar **buffPPtr, uchar *offset)
{
	num = num + 1;//plusOne
	u32 bitsLen = getBitsOfNum(num);
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	Append_g(bitsLen, &ptr, &off);
	num = num << (32 - bitsLen);
	num = num >> off;
	*buffPPtr = ptr + ((off + bitsLen) >> 3);
	*offset = (off + bitsLen) & 0x7;
	*(ptr++) += num >> 24 & 0xff;
	if (bitsLen < 16-off)
	{
		*ptr += (num >> 16) & 0xff;
		return;
	}
	if (bitsLen < 24 - off)
	{
		*(ptr++) += (num >> 16) & 0xff;
		*ptr += (num >> 8) & 0xff;
		return;
	}
	if (bitsLen <=32)
	{
		*(ptr++) += (num >> 16) & 0xff;
		*(ptr++) += (num >> 8) & 0xff;
		*ptr += num & 0xff;
		return;
	}
}
//test plusOne
#if 0
int main()
{
	int num = 1000000;
	u32 *runs = new u32[num];
	for (int i = 0; i < num; i++)
	{
		runs[i] = rand() % 100 + 1;
	}
	uchar *dst = new uchar[num * 3];
	memset(dst, 0, num * 3);
	uchar *dst_t = dst;
	uchar dstOff = 0;
	uchar *dst1 = new uchar[num * 3];
	memset(dst1, 0, num * 3);
	uchar *dst1_t = dst1;
	uchar dst1Off = 0;
	for (int i = 0; i < num; i++)
	{
		writePlusOne(runs[i], &dst_t, &dstOff);
	}
	for (int i = 0; i < num; i++)
	{
		plusOneCode(runs[i], &dst1_t, &dst1Off);
	}
	for (int i = 0; i < num * 3; i++)
	{
		if (dst[i] != dst1[i])
		{
			cout << i << "\t" << (u32)dst[i] << " " << (u32)dst1[i] << "\terror!" << endl;
		}
	}

	delete[] dst;
	delete[] dst1;
	return 0;
}
#endif

int gppHybirdCode(uchar *src, u32 bitsLen, uchar *dst, u16 HBblSize)
{
	int ret;
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}
	uchar *savedDst = dst;
	uchar *savedSrc = src;
	u32 srcNum = (bitsLen >> 3) + ((bitsLen & 0x7) ? 1 : 0);
	EndWords = src + srcNum;
	//get fist bit of src
	*dst = *src&(0x80);//将第一个bit写入压缩串
	bool flag = (bool)getNextOneBit(src, 0);//inmportant
	uchar dstOffset = 1;
	uchar srcOffset = 0;
	//记录上一个块结束位置
	uchar *lastSrcWord;
	uchar lastSrcOff;
	//Runs函数
	u32 *Runs = new u32[HBblSize];

	int i;
	while (src != EndWords)
	{
		u32 k = 0;
		u32 rbits = 0;
		int gapBits = 0;
		u32 sumBits = 0;
		lastSrcWord = src;
		lastSrcOff = srcOffset;
		while (rbits < HBblSize&&src != EndWords)
		{
			Runs[k] = getRuns(&src, &srcOffset);
			rbits += Runs[k];
			gapBits += getSubGP(Runs[k++]);
			flag = !flag;
		}
		if (k == 1 && HBblSize<rbits&&gapBits>0)
		{//plusOne is good
			writeMark2(&dst, &dstOffset,3);//plusOne
			writePlusOne(Runs[0], &dst, &dstOffset);
			continue;
		}
		for (i = 0; i < k; i++)//可以k-1，用来调试块大小
			sumBits += getBitsOfNum(Runs[i]);
		sumBits = (sumBits << 1) + k;
		if (src != EndWords)
		{
			if (sumBits < rbits)
			{
				if (gapBits <= 0)
				{
					writeMark2(&dst, &dstOffset, 2);//gamma
					for (i = 0; i < k; i++)
						Append_g(Runs[i], &dst, &dstOffset);
					continue;
				}
				writeMark2(&dst, &dstOffset, 3);//plusOne
				for (i = 0; i < k; i++)
					writePlusOne(Runs[i], &dst, &dstOffset);
				continue;
			}
			//调整flag
			if (rbits > HBblSize)
			{
				flag = !flag;//调整runs
				u32 bits_t = sumBits - HBblSize;	
			}	
			if(flag)
				writeMark2(&dst, &dstOffset, 1);//plain1
			else
				writeMark2(&dst, &dstOffset, 0);//plain0
			src = lastSrcWord;
			srcOffset == lastSrcOff;
			bitsCopy(&dst, &dstOffset, &src, &srcOffset, HBblSize);
			continue;
		}
		if (sumBits <= rbits)
		{
			if (gapBits <= 0)
			{
				writeMark2(&dst, &dstOffset, 4);//gamma
				for (i = 0; i < k; i++)
					Append_g(Runs[i], &dst, &dstOffset);
				continue;
			}
			writeMark2(&dst, &dstOffset, 3);//plusOne
			for (i = 0; i < k; i++)
				writePlusOne(Runs[i], &dst, &dstOffset);
			continue;

		}
		else{//这种情况应该极少出现
			writeMark2(&dst, &dstOffset, 0);//plain0
			src = lastSrcWord;
			srcOffset == lastSrcOff;
			bitsCopy(&dst, &dstOffset, &src, &srcOffset, rbits);
			cout << "极少情况出现！" << endl;
		}
	}
	return (dst - savedDst) * 8 + dstOffset;
}

//---------------------decompress-------------------------------------
u32 deCodeLen = 1;
u32 getMapBits(uchar *src,uchar srcOff)
{
	u32 val = src[0];
	if (src + 2 <= EndWords)
	{
		val = (val << 8) + src[1];
		val = (val << 8) + src[2];
		val = val << (8 + srcOff);
		return val >> 16;
	}
	else
	{
		if (src == EndWords)
			return val << 24;
		val = (val << 8) + src[1];
		return val << 16;
	}
	
}
int decodeGammaBlock(bool *flag, uchar **src, uchar *srcOff, uchar **dst, uchar *dstOff, u32 HBblSize)
{
	bool flagt = *flag;
	uchar *src_t = *src;
	uchar srcOff_t = *srcOff;
	//uchar *dst_t = *dst;
	//uchar dstOff_t = *dstOff;
	u32 deCodeLen = 0;
	u32 R1, R2, R4;
	while (src_t != EndWords)
	{
		u32 srcLen = 0;
		u32 mapVal = getMapBits(src_t, srcOff_t);
		R4 = (BitMap[mapVal] >> 24) & 0xff;
		BitMap[mapVal] = BitMap[mapVal] & (0xffff << R4);

		while (BitMap[mapVal])
		{
			R1 = BitMap[mapVal] & 0xff;
			deCodeLen += R1;
			R2 = (BitMap[mapVal] >> 8) & 0xff;
			srcLen += R2;
			if (flagt)
			{
				writeRuns(dst, dstOff, R1);
				flagt = !flagt;
			}
			if (HBblSize < deCodeLen)
			{
				*src = src_t + ((srcLen + srcOff_t) >> 3);
				*srcOff = (srcLen + srcOff_t) & 0x7;
				return 0;
			}
			BitMap[mapVal] = BitMap[mapVal] << R2;
		}
		src_t += (srcLen + srcOff_t) >> 3;
		srcOff_t = (srcLen + srcOff_t) & 0x7;
	}
	return 0;
}
#if 0
void Append_g(u32 runs, uchar **buffPPtr, uchar *offset)
{//runlength gamma
	uchar *ptr = *buffPPtr;
	uchar *ptrs = ptr;
	uchar off = *offset;
	uchar bitsLen = (getBitsOfNum(runs) << 1) + 1;
	ptr += (off + bitsLen) >> 3;
	*buffPPtr = ptr;
	off = (off + bitsLen) & 0x7;
	*offset = off;
	runs = runs << (8 - off);
	*ptr += runs & 0xff;
	if (ptr == ptrs)
		return;
	*(--ptr) += (runs >> 8) & 0xff;
	if (ptr == ptrs)
		return;
	*(--ptr) += (runs >> 16) & 0xff;
	if (ptr == ptrs)
		return;
	*(--ptr) += (runs >> 24) & 0xff;
	if (ptr == ptrs)
		return;
}
int elisGammaCode1(u32 num, uchar **buffPPtr, uchar *offset)
{
	int ret;
	//if (!num || !buffPPtr || !offset)
	//{
	//	return -1;
	//}
	uchar *ptr = *buffPPtr;
	uchar off = *offset;

	u32 bitsLen = getBitsOfNum(num);
	if (bitsLen == -1)
	{
		return -1;
	}

	u32 i;

	//can't mark off this segments
	for (i = 0; i<bitsLen; i++)
	{
		*ptr &= ~(1 << (7 - off));
		off++;
		if (off == 8)
		{
			off = 0;
			ptr++;
		}
	}
	*ptr |= 1 << (7 - off);
	if (++off == 8)
	{
		off = 0;
		ptr++;
	}

	for (i = 1; i <= bitsLen; i++)
	{
		if (num & (1 << (bitsLen - i)))
		{
			*ptr |= (1 << (7 - off));
		}
		else
		{
			*ptr &= ~(1 << (7 - off));
		}

		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
	}
	*buffPPtr = ptr;
	*offset = off;
	return 0;

}
//写入Runs的测试程序

int main()
{
	int Nums = 1000;
	int Rnums = Nums >> 1;
	u32 *Runs = new u32[Rnums];
	//存储结构
	uchar *dst = new uchar[Nums];
	memset(dst, 0, Nums);
	uchar *dst_t = dst;
	uchar dOff = 0;

	uchar *dst1 = new uchar[Nums];
	memset(dst1, 0, Nums);
	uchar *dst1_t = dst1;
	uchar d1Off = 0;

	int sumRuns = 0;
	for (int i = 0; i < Rnums; i++)
	{
		Runs[i] = rand() % 34 + 1;
		sumRuns += Runs[i];
	}
	
	//存储1
	/*for (int i = 0; i < Rnums; i++)
	{
	elisGammaCode1(Runs[i], &dst_t, &dOff);
	Append_g(Runs[i], &dst1_t, &d1Off);
	}*/
	
	for (int i = 0; i < Rnums; i++)
	{
		elisGammaCode1(Runs[i], &dst_t, &dOff);
	}

	for (int i = 0; i < Rnums; i++)
	{
		Append_g(Runs[i], &dst1_t, &d1Off);
	}
	int k = 0;
	while (dst[k] == dst1[k] && k < Nums)
	{
		k++;
	}
	//解压测试

	delete[]Runs;
	delete[]dst;
	delete[]dst1;
	return 0;
}
#endif
int deGppHybirdCode(uchar *src, u32 bitsLen, uchar *dst, u16 HBblSize)
{//src is compressed string ，dst is decompressed string
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}
	uchar srcOffset = 1;
	uchar dstOffset = 0;

	uchar *savedSrc = src;
	uchar *savedDst = dst;
	u32 srcNum = (bitsLen >> 3) + ((bitsLen & 0x7) ? 1 : 0);
	EndWords = src + srcNum;
	bool flag = src[0] & 128 ? true : false;
	deCodeLen = 1;
	while (src < EndWords)
	{
		uchar HeadMark = getMark2(&src, &srcOffset);
		deCodeLen += 2;
		switch (HeadMark)
		{
		case 0:
			flag = false;
			if (bitsLen - deCodeLen > HBblSize)
			{
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, HBblSize);
				deCodeLen += HBblSize;
			}
			else{
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, bitsLen-deCodeLen);
				deCodeLen += bitsLen - deCodeLen;
			}
			break;
		case 1:
			flag = true;
			if (bitsLen - deCodeLen > HBblSize)
			{
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, HBblSize);
				deCodeLen += HBblSize;
			}
			else{
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, bitsLen - deCodeLen);
				deCodeLen += bitsLen - deCodeLen;
			}
			break;
		case 2://decode gamma
			decodeGammaBlock(&flag, &src, &srcOffset, &dst, &dstOffset, HBblSize);
			break;
		case 3:
			break;
		default:
			cout << "compressed string error!" << endl;
			break;
		}

	}
	return (dst - savedDst) * 8 + dstOffset;
}
#if 1 
void writeOneBits(uchar *src, u32 index)
{
	u32 swords = index >> 3;
	u32 offset = index & 0x7;
	src[swords] += 128 >> offset;
}
int creatSrc(uchar *src, u32& bitsLen)
{
	if (!src)
	{
		cout << "the parameter（src） is error!" << endl;
		return -1;
	}
	string str="000000000000110011010000000111111111100000000000000000000000000000000000";
	//cin >> str;
	int i = 0;
	while (bitsLen<str.length())
	{
		if (str[bitsLen] != '0')
		{
			writeOneBits(src, bitsLen);
		}
		bitsLen++;
	}
	return 0;
}
int main()
{
	int num = 1000;
	u32 HYbsize = 16;
	uchar *src = new uchar[num];
	memset(src, 0, num);
	u32 bitsLen = 0;
	creatSrc(src, bitsLen);
	cout << endl;
	printBitsForArray(src, 0, bitsLen);
	uchar *dst = new uchar[num * 2];
	memset(dst, 0, num * 2);
	u32 dstLen=gppHybirdCode(src, bitsLen, dst, 8);
	printBitsForArray(dst, 0, dstLen);
	return 0;
}
#endif
