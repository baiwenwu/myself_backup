#include"testFunction.h"
#include"oneplus.h"
#include <vector>
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
		if (i > 65530)
		{
			int xxx = 0;
		}
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

	int xxxx = 0;
	cout <<"mapBits"<< endl;
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
	*(ptr++) += (num >> 24) & 0xff;
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
u32 *Runs = NULL;
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
	EndOff=bitsLen&0x7;
	//get fist bit of src
	*dst = *src&(0x80);//将第一个bit写入压缩串
	bool flag = (bool)getNextOneBit(src, 0);//inmportant
	uchar dstOffset = 1;
	uchar srcOffset = 0;
	//记录上一个块结束位置
	uchar *lastSrcWord;
	uchar lastSrcOff;
	//Runs函数
	int i;
	while (src < EndWords)
	{
		u32 k = 0;
		u32 rbits = 0;
		int gapBits = 0;
		u32 sumBits = 0;
		lastSrcWord = src;
		lastSrcOff = srcOffset;
		while (rbits < HBblSize&&src < EndWords)
		{
			Runs[k] = getRuns(&src, &srcOffset);
			if (Runs[k] == 0)break;
			rbits += Runs[k];
			gapBits += getSubGP(Runs[k++]);
			flag = !flag;
		}
		if(k==0)
			break;
		if (k == 1 && HBblSize<rbits)
		{//plusOne is good
			writeMark2(&dst, &dstOffset,3);//plusOne
			writePlusOne(Runs[0], &dst, &dstOffset);
			continue;
		}
		for (i = 0; i < k; i++)//可以k-1，用来调试块大小
			sumBits += getBitsOfNum(Runs[i]);
		sumBits = (sumBits << 1) + k;
		if (src < EndWords)
		{
			if (sumBits <= rbits)
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
				//u32 bits_t = sumBits - HBblSize;	
			}	
			if(flag)
				writeMark2(&dst, &dstOffset, 1);//plain1
			else
				writeMark2(&dst, &dstOffset, 0);//plain0
			src = lastSrcWord;
			srcOffset = lastSrcOff;
			bitsCopy(&dst, &dstOffset, &src, &srcOffset, HBblSize);
			continue;
		}
		if (sumBits <= rbits + 7)
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
		else{//这种情况应该极少出现
			writeMark2(&dst, &dstOffset, 0);//plain0
			src = lastSrcWord;
			srcOffset =lastSrcOff;
			if (rbits>HBblSize)rbits = HBblSize;
			bitsCopy(&dst, &dstOffset, &src, &srcOffset, rbits);
			//src=EndWords;
			//cout << "\n极少情况出现！" << endl;
		}
	}
	if (srcOffset < EndOff)
	{
		int xxxx = 0;
	}
	return (dst - savedDst) * 8 + dstOffset;
}

//---------------------decompress-------------------------------------
u32 deCodeLen = 1;
int decodePlusOne_t(bool *flag, uchar **src, uchar *srcOff,
	uchar **dst, uchar *dstOff, u32 HBblSize)
{
	bool flagt = *flag;
	uchar *src_t = *src;
	uchar srcOff_t = *srcOff;
	u32 deCodeLen = 0;
	u32 R1, R2;
	while (src_t<EndWords)
	{
		u32 Runs = 1;
		u16 poVal_t = getBitsPO(src_t, srcOff_t);
		u16 poVal = poVal_t >> 7;
		R2 = (plusOlen[poVal] >> 8) & 0xff;
		R1 = plusOlen[poVal] & 0xff;
		if (R1 < 32){
				Runs = Runs << R1;
				src_t += (srcOff_t + R2) >> 3;
				srcOff_t = (srcOff_t + R2) & 0x7;
				Runs += getBitsPO1(src_t, srcOff_t, R1);
				src_t = src_t + ((srcOff_t + R1) >> 3);
				srcOff_t = (srcOff_t + R1) & 0x7;
				Runs--;
		}
		else
		{	
			Runs = R1 - 33;
			src_t += (srcOff_t + R2) >> 3;
			srcOff_t = (srcOff_t + R2) & 0x7;
		}
		if (flagt)
		{
			writeRuns(dst, dstOff, Runs);
		}
		else
		{
			*dst = *dst + ((*dstOff + Runs) >> 3);
			*dstOff = (*dstOff + Runs) & 7;
		}
		flagt = !flagt;
		deCodeLen += Runs;
		if (HBblSize <= deCodeLen)
		{
			*flag = flagt;
			*src = src_t;
			*srcOff =srcOff_t;
			return 0;
		}
	}
	while(srcOff_t < EndOff)
	{
		u16 poVal_t = getBitsPO(src_t, srcOff_t);
		u16 poVal = poVal_t >> 7;
		R2 = (plusOlen[poVal] >> 8) & 0xff;
		R1 = (plusOlen[poVal] & 0xff) - 33;
		if (flagt)
		{
			writeRuns(dst, dstOff, R1);
		}
		else
		{
			*dst = *dst + ((*dstOff + R1) >> 3);
			*dstOff = (*dstOff + R1) & 7;
		}
		flagt = !flagt;
		srcOff_t = (srcOff_t + R2) & 0x7;
		deCodeLen += R1;
		if (HBblSize <= deCodeLen)
		{
			*flag = flagt;
			*src = src_t;
			*srcOff = srcOff_t;
			return 0;
		}
	}
	*srcOff = EndOff;
	*src = EndWords + 1; 
	return 0;
}
int decodePlusOne(bool *flag, uchar **src, uchar *srcOff,
	uchar **dst, uchar *dstOff, u32 HBblSize)
{
	uchar *dst_t = *dst;
	uchar dstOff_t = *dstOff;
	bool flagt = *flag;
	uchar *src_t = *src;
	uchar srcOff_t = *srcOff;
	u32 deCodeLen = 0;
	u32 R1, R2;
	while (src_t<EndWords)
	{
		u32 Runs = 1;
		u16 val = *src_t;
		val = (val << 8) | src_t[1];
		val = val << srcOff_t;
		u16 poVal = val >> 7;
		R2 = (plusOlen[poVal] >> 8) & 0xff;
		R1 = plusOlen[poVal] & 0xff;
		if (R1 < 32){
			Runs = Runs << R1;
			src_t += (srcOff_t + R2) >> 3;
			srcOff_t = (srcOff_t + R2) & 0x7;
			Runs += getBitsPO1(src_t, srcOff_t, R1);
			src_t = src_t + ((srcOff_t + R1) >> 3);
			srcOff_t = (srcOff_t + R1) & 0x7;
			Runs--;
		}
		else
		{
			Runs = R1 - 33;
			src_t += (srcOff_t + R2) >> 3;
			srcOff_t = (srcOff_t + R2) & 0x7;
		}
		if (flagt)
		{
			writeRuns_t(dst_t, dstOff_t, Runs);
		}
		dst_t = dst_t + ((dstOff_t + Runs) >> 3);
		dstOff_t = (dstOff_t + Runs) & 7;
		
		flagt = !flagt;
		deCodeLen += Runs;
		if (HBblSize <= deCodeLen)
		{
			*flag = flagt;
			*dst = dst_t;
			*dstOff = dstOff_t;
			*src = src_t;
			*srcOff = srcOff_t;
			return 0;
		}
	}
	while (srcOff_t < EndOff)
	{
		u16 val = *src_t;
		val = (val << 8) | src_t[1];
		val = val << srcOff_t;
		u16 poVal = val >> 7;
		R2 = (plusOlen[poVal] >> 8) & 0xff;
		R1 = (plusOlen[poVal] & 0xff) - 33;
		if (flagt)
		{
			writeRuns_t(dst_t, dstOff_t, R1);
		}
		dst_t = dst_t + ((dstOff_t + R1) >> 3);
		dstOff_t = (dstOff_t + R1) & 7;
		flagt = !flagt;
		srcOff_t = (srcOff_t + R2) & 0x7;
		deCodeLen += R1;
		if (HBblSize <= deCodeLen)
		{
			*flag = flagt;
			*src = src_t;
			*srcOff = srcOff_t;
			*dst = dst_t;
			*dstOff = dstOff_t;
			return 0;
		}
	}
	*dst = dst_t;
	*dstOff = dstOff_t;
	*srcOff = EndOff;
	*src = EndWords + 1;
	return 0;
}
u32 getMapBits(uchar *src, uchar srcOff)
{
	u32 val = src[0];
	if (src + 2 <= EndWords)
	{
		val = (val << 8) + src[1];
		val = (val << 8) + src[2];
		//val=val<<srcOff;
		val = val << (8 + srcOff);
		return val >> 16;
	}
	else
	{
		if (src == EndWords)
			return (val << (8 + srcOff)) & 0xffff;
		val = (val << 8) + src[1];
		return (val << srcOff) & 0xffff;
	}

}
int decodeGammaBlock_t(bool *flag, uchar **src, uchar *srcOff,
	 uchar **dst, uchar *dstOff, u32 HBblSize)
{//用大表的方法解码时需要多给压缩传分配2-3个字节的空间，防止出现错误
	bool flagt = *flag;
	uchar *src_t = *src;
	uchar srcOff_t = *srcOff;
	//uchar *dst_t = *dst;
	//uchar dstOff_t = *dstOff;
	u32 deCodeLen = 0;
	u32 R1, R2, R4;
	while(src_t<EndWords)
	{
		/*if (src_t == EndWords - 1 && srcOff_t == EndOff)
			break;*/
		u32 srcLen = 0;
		u32 mapVal = getMapBits(src_t, srcOff_t);
		R4 = (BitMap[mapVal] >> 24) & 0xff;
 		mapVal = mapVal & (0xffff << R4);

		while (mapVal)
		{
			R1 = BitMap[mapVal] & 0xff;
 			deCodeLen += R1;
			R2 = (BitMap[mapVal] >> 8) & 0xff;
			srcLen += R2;
			if (flagt)
			{
				writeRuns(dst, dstOff, R1);
			}
			else
			{
				*dst=*dst +((*dstOff+R1)>>3);
				*dstOff=(*dstOff+R1)&7;
			}
			flagt = !flagt;
			if (HBblSize <= deCodeLen)
			{
				 *flag=flagt;
				*src = src_t + ((srcLen + srcOff_t) >> 3);
				*srcOff = (srcLen + srcOff_t) & 0x7;
				return 0;
			}
			mapVal = (mapVal << R2)&0xffff;
		}
		src_t += (srcLen + srcOff_t) >> 3;
		srcOff_t = (srcLen + srcOff_t) & 0x7;
	}
	if(srcOff_t<EndOff)
	{
		u32 mapVal = getMapBits(src_t, srcOff_t);
		while (mapVal)
		{
			R1 = BitMap[mapVal] & 0xff;
			R2 = (BitMap[mapVal] >> 8) & 0xff;
			if (flagt)
			{
				writeRuns(dst, dstOff, R1);
			}
			else
			{
				*dst=*dst +((*dstOff+R1)>>3);
				*dstOff=(*dstOff+R1)&7;
			}
			flagt = !flagt;
			//bug is here
			deCodeLen += R1;
			srcOff_t=(R2 + srcOff_t) & 0x7;
			if (HBblSize <= deCodeLen&&srcOff_t<EndOff)
			{
				*flag = flagt;
				*src = src_t;
				*srcOff = srcOff_t;
				return 0;
			}
			mapVal = (mapVal << R2)&0xffff;
		}
		
	}
	*srcOff = EndOff;
	*src = EndWords + 1;
	return 0;
}
int decodeGammaBlock(bool *flag, uchar **src, uchar *srcOff,
	uchar **dst, uchar *dstOff, u32 HBblSize)
{
	uchar *dst_t = *dst;
	uchar dstOff_t = *dstOff;
	bool flagt = *flag;
	uchar *src_t = *src;
	uchar srcOff_t = *srcOff;
	u32 deCodeLen = 0;
	u32 R1, R2, Runs;
	while (src_t<EndWords)
	{
		u16 val = *src_t;
		val = (val << 8) | src_t[1];
		val = val << srcOff_t;
		u16 val_t = val >> 7;
		R2 = degaTab[val_t] >> 8;
		Runs = degaTab[val_t] & 0xff;

		if (Runs)
		{
			if (flagt)
				writeRuns_t(dst_t, dstOff_t, Runs);
		}
		else
		{
			if (R2 < 9){
				src_t += (srcOff_t + R2) >> 3;
				srcOff_t = (srcOff_t + R2) & 0x7;
				R2++;
				Runs = getBitsPO1(src_t, srcOff_t, R2);
			}
			else{
				R2 = getRunsForDeg(&src_t, &srcOff_t);
				R2++;
				Runs = getBitsPO1(src_t, srcOff_t, R2);
			}
			if (flagt)
				writeRuns_t(dst_t, dstOff_t, Runs);
		}
		flagt = !flagt;
		dst_t += (dstOff_t + Runs) >> 3;
		dstOff_t = (dstOff_t + Runs) & 0x7;
		src_t += (srcOff_t + R2) >> 3;
		srcOff_t = (srcOff_t + R2) & 0x7;
		deCodeLen += Runs;
		if (HBblSize <= deCodeLen)
		{
			*flag = flagt;
			*src = src_t;
			*srcOff = srcOff_t;
			*dst = dst_t;
			*dstOff = dstOff_t;
			return 0;
		}
	}
	while (srcOff_t<EndOff)
	{
		u16 val = *src_t;
		val = (val << 8) | src_t[1];
		val = val << srcOff_t;
		u16 val_t = val >> 7;
		R2 = degaTab[val_t] >> 8;
		Runs = degaTab[val_t] & 0xff;
		if (Runs){
			if (flagt)
				writeRuns_t(dst_t, dstOff_t, Runs);
		}
		dst_t += (dstOff_t + Runs) >> 3;
		dstOff_t = (dstOff_t + Runs) & 0x7;
		deCodeLen += Runs;
		srcOff_t += R2;
		flagt = !flagt;
		if (HBblSize <= deCodeLen&&srcOff_t<EndOff)
		{
			*flag = flagt;
			*src = src_t;
			*srcOff = srcOff_t;
			*dst = dst_t;
			*dstOff = dstOff_t;
			return 0;
		}
	}
	*dst = dst_t;
	*dstOff = dstOff_t;
	*srcOff = EndOff;
	*src = EndWords + 1;
	return 0;
}
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
	u32 srcNum = (bitsLen >> 3) + ((bitsLen & 0x7) ? 1 : 1);
	uchar*endWS = src + srcNum;
	EndWords = endWS - 1;
	EndOff = bitsLen & 0x7;
	bool flag = src[0] & 128 ? true : false;
	deCodeLen = 1;
	//uchar endOff = EndOff ? EndOff : 8;
	int xx = 0;
	while (src < endWS)
	{
		if (src == EndWords&&srcOffset == EndOff)break;
		uchar HeadMark = getMark2(&src, &srcOffset);
		switch (HeadMark)
		{
		case 0:
			flag = false;
			deCodeLen = (EndWords - src) * 8 + EndOff - srcOffset;
			if (deCodeLen>bitsLen)
			{
				src = EndWords;
				break;
			}
			if (deCodeLen > HBblSize)
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, HBblSize);	
			else
			{
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, deCodeLen);
				src = EndWords;
			}	
			break;
		case 1:
			flag = true;
			deCodeLen = (EndWords - src) * 8 + EndOff - srcOffset;
			if (deCodeLen > HBblSize)
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, HBblSize);
			else
			{
				bitsCopy(&dst, &dstOffset, &src, &srcOffset, deCodeLen);
				src = EndWords;
			}
			break;
		case 2://decode gamma
			decodeGammaBlock(&flag, &src, &srcOffset, &dst, &dstOffset, HBblSize);
			break;
		case 3:
			decodePlusOne(&flag, &src, &srcOffset, &dst, &dstOffset, HBblSize);
			break;
		default:
			cout << "compressed string error!" << endl;
			break;
		}

	}
	return (dst - savedDst) * 8 + dstOffset;
}

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
	string str = "1111111111111111111110000000011111111111100001111111110000000000000000000001100001111000000001010101011111010000000000000111";
	//string str="00000000000011001101000000011111111110000000000000000000000000000000000001100000000101000011111011111101101001010000000000011111111111111111111010000001001001111111111111111111111100000000000011111010111001001010010100011100110011";
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
int creatSrcRand(uchar *src, u32& bitsLen,u32 num)
{
	if (!src)
	{
		cout << "the parameter（src） is error!" << endl;
		return -1;
	}
	int range = 8;
	int addRan = 1;
	int anvRuns = 0;
	uchar *endW = src + num;
	uchar *startW = src+1;
	u32 bitslen = 0;
	u32 randNum = rand() % range + addRan;
	uchar *src_t = src;
	uchar srcOff = 0;
	bool flag = true;
	do{
		bitslen += randNum;
		anvRuns++;
		if (flag)
			writeRuns(&src_t, &srcOff, randNum);
		else
		{
			src_t += (srcOff + randNum) >> 3;
			srcOff = (srcOff + randNum) & 0x7;
		}
		flag = !flag;
		randNum = rand() % range + addRan;
		startW = src + 1;
		startW += (bitslen+ randNum) >> 3;
	} while (startW < endW);
	bitsLen = bitslen;
	//cout << "\n平均 AvRuns=" << (double)bitslen / anvRuns << endl;
	return 0;
}
string saveErorrBits(uchar *src, uchar off, u32 bitsLen)
{
	string str;
	u32 bitslen_t = 0;
	while (bitslen_t < bitsLen)
	{
		u32 words = bitslen_t >> 3;
		uchar offset = bitslen_t & 0x7;
		if (src[words] & (128 >> offset))
			str += '1';
		else
			str += '0';
		bitslen_t++;
	}
	return str;
}

#if 1
//混合编码测试
int main1_t(u32 num1,int x,u32 BlckSize1)
//int main()
{
	//CreateBitMap();

	u32 blockSize = 64;
	int num = 1000;
	num = num1;
	u32 HYbsize = 16;
	blockSize = BlckSize1;
	uchar *src = new uchar[num];
	memset(src, 0, num);
	u32 bitsLen = 0;
	if (x==1)
		creatSrc(src, bitsLen);
	else
		creatSrcRand(src, bitsLen, num);
	//creatSrc(src, bitsLen);
	if (x)
	{
		cout << endl;
		printBitsForArray(src, 0, bitsLen);
		cout << "\n未压缩时长度：" << bitsLen << endl;
	}
	//cout << "块大小：" << blockSize << endl;
	//cout << "\n未压缩时长度：" << bitsLen << endl;
	uchar *dst = new uchar[num * 2];
	memset(dst, 0, num * 2);
	Runs = new u32[BlckSize1];
	u32 dstLen = gppHybirdCode(src, bitsLen, dst, blockSize);
	if (x)
	{
		cout << endl;
		printBitsForArray(dst, 0, dstLen);
		cout << "\n压缩后长度：" << dstLen << endl;
	}
	//cout << "\n压缩后长度：" << dstLen << endl;
	//cout << "压缩率" << double(dstLen) / double(bitsLen) << endl;
	uchar *src1= new uchar[num];
	memset(src1, 0, num);
	
	u32 src1Len = deGppHybirdCode(dst, dstLen, src1, blockSize);
	if (x)
	{
		printBitsForArray(src1, 0, src1Len);
		cout << "\n解压后长度：" << src1Len << endl;
	}
	//cout << "\n解压后长度：" << src1Len << endl;
	int ret = 0;
	for (int i = 0; i < num; i++)
	{
		if (src1[i] != src[i])
		{
			cout <<"\n"<< i << "  error!" << endl;
			ret = -1;
		}
	}
	
	if (bitsLen != src1Len)
	{
		cout << "\n解压出来的长度不一致！" << endl;
		cout << "原长：" << bitsLen << "\t解压长：" << src1Len << endl;
		ret = -1;
	}
	if (ret == -1)
	{
		if (!x)
		{
			cout << "源串：" << endl;
			//保存源串
			saveErorrBits(src, 0, bitsLen);
			printBitsForArray(src, 0, bitsLen);
			cout << "\n压缩串：" << endl;
			printBitsForArray(dst, 0, dstLen);
			cout << "\n解压串：" << endl;
			printBitsForArray(src1, 0, src1Len);
			int xxx = 0;
		}
	}
	delete[]src;
	delete[]dst;
	delete[]src1;
	return ret;
}
int main12()
{
	u32 HB = 16;
	CreateBitMap();
	for (int kj = 7; kj < 100000; kj++)
	{
		cout <<" "<< kj;
		for (u32 ki = 2; ki < 1000; ki++)
		{
			if (kj == 7 && ki== 12)
			{
				if (main1_t(kj, 2, HB) == -1)
					cout << "\n-------------------" << kj << "- " << ki << "--------------" << endl;
				//return 0;
			}
			else
			{
				if (main1_t(kj, 0, HB) == -1)
					cout << "\n-------------------" << kj << "- " << ki << "--------------" << endl;
				//return 0;
			}
			
		}
	}
	return 0;
}
int main()
{
	//CreateBitMap();
	u32 HB = 8;
	while (HB < 9000)
	{
		main1_t(10000000, 0, HB);
		HB = HB * 2;
	}
	//main12();
	//main1_t(1000, 1,8);
	return 0;
}
#endif
#if 0
int runLengthGammaCode(uchar *src, u32 bitsLen, uchar *dst)
{
	int ret;
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}
	uchar *savedDst = dst;

	//get fist bit of src
	*dst = *src;
	uchar offset = 1;
	bool flag;
	if (*src &(1 << 7))
	{
		flag = true;
	}
	else
	{
		flag = false;
	}

	u32 i;
	u32 period = 1;

	bool flag1, flag2;
	for (i = 1; i<bitsLen; i++)
	{
		flag1 = (src[i / 8] & (1 << (7 - i % 8)))
			&&
			flag
			;
		flag2 = (src[i / 8] & (1 << (7 - i % 8)))
			||
			flag
			;
		if (flag1 || !flag2)
		{
			period++;
		}
		else
		{

			ret = elisGammaCode(period, &dst, &offset);
			if (ret<0)
			{
				return ret;
			}
			period = 1;//reset the length of run
			flag = flag ? false : true;// switch the mark of runs
		}

	}

	elisGammaCode(period, &dst, &offset);

	return (dst - savedDst) * 8 + offset;
}
int main()
{
	int num = 1000000;
	u32 HYbsize = 16;
	uchar *src = new uchar[num];
	memset(src, 0, num);
	u32 bitsLen = 0;
	creatSrcRand(src, bitsLen,num);
	//creatSrc(src, bitsLen);
	cout << "\n源串：" << bitsLen<<endl;
	//printBitsForArray(src, 0, bitsLen);
	uchar *dst = new uchar[num * 2];
	memset(dst, 0, num * 2);
	u32 dstLen=runLengthGammaCode(src, bitsLen,dst);
	cout << "\n压缩完成后:" << dstLen<<endl;
	//printBitsForArray(dst, 0, dstLen);


	//CreateBitMap();
    EndWords=dst+(dstLen>>3)+(dstLen&0x7?1:1);
	EndWords--;
	EndOff=dstLen&0x7;
	bool flag=false;
	if(dst[0]&128)
	{
		flag=true;
	}
	uchar *dst_t=dst;
	uchar dstOff_t=1;

	uchar *src1 = new uchar[num];
	memset(src1, 0, num);
	uchar *src1_t=src1;
	u32 srcLen = 0;
	//srcLen=runLengthGammaDecode_228(dst, dstLen, src1);
	uchar src1Off=0;
	
	while(dst_t<EndWords+1)
	{
		if (EndWords - dst_t < 2)
		{
			int xxx = 0;
		}
		decodeGammaBlock(&flag, &dst_t, &dstOff_t,
			&src1_t, &src1Off, 16);
	}
	srcLen = (src1_t - src1) * 8 + src1Off;
	cout << "\n 解压后: " << srcLen<<endl;
	//printBitsForArray(src1, 0, srcLen);
	cout << endl;
	for (int i = 0; i < num; i++)
	{
		if (src[i] != src1[i])
		{
			cout << "i" << " " << (int)src[i] << " " << (int)src1[i] << "  error!" << endl;
			int j = i - 4;
			printBitsForArray(src, j * 8, srcLen - j * 8);
			cout << endl;
			printBitsForArray(src1, j * 8, srcLen - j * 8);
			break;
		}
	}
	return 0; 
}
#endif

#if 0
//测试plusOne编码
int main()
{
	u32 blockSize = 256;
	int num = 1000000;
	u32 HYbsize = 16;
	uchar *src = new uchar[num];
	memset(src, 0, num);
	u32 bitsLen = 0;
	creatSrcRand(src, bitsLen, num);
	//creatSrc(src, bitsLen);
	cout << "\n源串：" << endl;
	//printBitsForArray(src, 0, bitsLen);
	uchar *dst = new uchar[num * 2];
	memset(dst, 0, num * 2);
	u32 dstLen = runLengthPlusOneCode(src, bitsLen, dst);
	cout << "\n压缩完成后" << endl;
	//printBitsForArray(dst, 0, dstLen);


	EndWords = dst + (dstLen >> 3) + (dstLen & 0x7 ? 1 : 0);
	EndWords--;
	EndOff = dstLen & 0x7;
	bool flag = false;
	if (dst[0] & 128)
	{
		flag = true;
	}
	uchar *dst_t = dst;
	uchar dstOff_t = 1;

	uchar *src1 = new uchar[num];
	memset(src1, 0, num);
	uchar *src1_t = src1;
	uchar src1Off = 0;
	u32 srcLen = 0;
	int xnum = 0;
	while (dst_t<EndWords + 1)
	{
		/*xnum++;
		if (xnum == 31)
		{
			int xxxx =0;
		}*/
		decodePlusOne(&flag, &dst_t, &dstOff_t,
			&src1_t, &src1Off, blockSize);
	}
	srcLen = (src1_t - src1) * 8 + src1Off;
	cout << "\n 解压后" << endl;
	//printBitsForArray(src1, 0, srcLen);
	cout << endl;
	for (int i = 0; i < num; i++)
	{
		if (src[i] != src1[i])
		{
			cout << "i" << " " << (int)src[i] << " " << (int)src1[i] << "  error!" << endl;
		}
	}
	return 0;
}
#endif

u16 dedeTab[512] = {
	2304, 2048, 1792, 1792, 1536, 1536, 1536, 1536,	1280, 1280, 1280, 1280, 1280, 1280, 1280, 1280,
	2319, 2320, 2321, 2322, 2323, 2324, 2325, 2326,	2327, 2328, 2329, 2330, 2331, 2332, 2333, 2334,
	1799, 1799, 1799, 1799, 1800, 1800, 1800, 1800,	1801, 1801, 1801, 1801, 1802, 1802, 1802, 1802,
	1803, 1803, 1803, 1803, 1804, 1804, 1804, 1804,	1805, 1805, 1805, 1805, 1806, 1806, 1806, 1806,
	2088, 2088, 2089, 2089, 2090, 2090, 2091, 2091,	2092, 2092, 2093, 2093, 2094, 2094, 2095, 2095,
	2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359,	2360, 2361, 2362, 2363, 2364, 2365, 2366, 2367,
	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,
	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,
	1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058,	1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058,
	1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058,	1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058,
	1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059,	1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059,
	1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059,	1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059,
	1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316,	1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316,
	1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317,	1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317,
	1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318,	1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318,
	1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319,	1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289,	289, 289, 289, 289, 289, 289, 289, 289
	};
#if 0
//新gamma解码加速生成
void printUchar_t(uchar x)
{
	uchar andx = 128;
	while (andx)
	{
		if (andx&x)
			cout << "1";
		else
			cout << "0";
		andx = andx >> 1;
	}
	cout << " ";
}
uchar findZeroU16_9(u16 x)
{
	u16 andx = 256;
	u16 runs = 0;
	while (andx)
	{
		if (andx&x)
			break;
		else
			runs++;
		andx = andx >> 1;
	}
	return runs;
}
int main()
{
	u16 R1, R2;
	R1 = R2 = 0;
	for (u32 i = 0; i < 512; i++)
	{
		R1 = 0;
		R2 = findZeroU16_9(i);
		if (R2 < 5)
		{
			R1 = i >> (8 - 2 * R2);
			R1--;
			R2 += R2 + 1;
			if (9 - R2 >= R1)
			{
				u16 tmp = 1 << R1;
				tmp += (i>>(9-R1-R2))&(0x1ff >> (9-R1));
				R2 = R2 + R1;
				R1 = tmp+32;
			}
		}
		dedeTab[i] = R2 << 8;
		dedeTab[i] += R1;
		printUchar_t(i >> 8);
		printUchar_t(i & 0xff);
		cout << "\t" << R2 << "\t" << R1 <<"\t";
		printUchar_t(dedeTab[i] >> 8);
		printUchar_t(dedeTab[i] & 0xff);
		cout << endl;
	}
	cout << endl;
	
	ofstream out;
	out.open("dedeTab.txt", ios::out);
	if (!out.is_open())
	{
		cout << "open file failed!" << endl;
		exit(0);
	}
	for (u32 i = 0; i < 512; i++)
	{
		if (i % 8 == 0)
		{
			out << endl;
		}
		out << (int)dedeTab[i] << ",";
		R2 = dedeTab[i] >> 8;
		R1 = dedeTab[i] & 0xff;
		printUchar_t(i >> 8);
		printUchar_t(i & 0xff);
		cout << "\t" << R2 << "\t" << R1 << "\t" << endl;
		//printUchar_t(degaTab[i] >> 8);
		//printUchar_t(degaTab[i] & 0xff);
		
	}
	out.close();
	return 0;

}

#endif
