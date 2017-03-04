#include"interface_g.h"
#include<fstream>
int bitsNumTbls[256] = {
	-1, 0, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
};
int getBitsOfNum(unsigned x)
{
	if (x<256){
		return bitsNumTbls[x];
	}
	int n = 1;
	if (x == 0) return -1;
	if ((x >> 16) == 0) { n = n + 16; x = x << 16; }
	if ((x >> 24) == 0) { n = n + 8; x = x << 8; }
	if ((x >> 28) == 0) { n = n + 4; x = x << 4; }
	if ((x >> 30) == 0) { n = n + 2; x = x << 2; }
	n = n - (x >> 31);
	return 31 - n;
}
uchar getOneUchar(uchar *src, u32 index)
{
	u32 words = index >> 3;
	uchar offset = index & 0x7;
	uchar ch_t = src[words++] << offset;
	ch_t += src[words] >> (8 - offset);
	return ch_t;
}



void bitsCopyIndex(uchar *dst, u32 dst_index, uchar * src, u32 src_index, u32 len)
{
	u32 numBits = 0;
	u32 src_words = src_index >> 3;
	uchar src_offset = src_index & 0x7;
	u32 dst_words = dst_index >> 3;
	uchar dst_offset = dst_index & 0x7;
	uchar offset;
	numBits += (8 - src_offset);
	if (src_offset >= dst_offset)
	{
		uchar ch_t = src[src_words] << src_offset;
		dst[dst_words] += (ch_t >> dst_offset);
		offset = 8 - (src_offset - dst_offset);
	}
	else
	{
		uchar ch_t = src[src_words] << src_offset;
		dst[dst_words++] += (ch_t >> dst_offset);
		offset = dst_offset - src_offset;
		dst[dst_words] += (ch_t << (8 - dst_offset));
	}
	if (offset == 8)
	{
		if (src_offset)
		{
			dst_words++;
			src_words++;
		}
		for (u32 i = 0; i < (len >> 3); i++)
			dst[dst_words++] = src[src_words++];
		u32 word_t = (src_index + len) >> 3;
		dst[word_t] = src[word_t];
		src_offset = (src_index + len) & 0x7;
		dst[word_t] = dst[word_t] >> (8 - src_offset);
		dst[word_t] = dst[word_t] << (8 - src_offset);
		return;
	}
	while (numBits < len)
	{
		uchar ch_t = src[++src_words];
		dst[dst_words++] += (ch_t >> offset);
		dst[dst_words] = (ch_t << (8 - offset));
		numBits += 8;
	}
	dst_words = (dst_index + len) >> 3;
	uchar dstoff_t = (dst_index + len) & 0x7;
	uchar cd_ch_t = dst[dst_words] >> (8 - dstoff_t);
	dst[dst_words++] = cd_ch_t << (8 - dstoff_t);
	dst[dst_words] = 0;
}
void bitsCopy1(uchar *dst, uchar dstoff, uchar *src, uchar srcOff, u32 len)
{
	uchar *endDstWord = dst + ((len + dstoff) >> 3);
	uchar dstoff_t = (len + dstoff) & 0x7;
	if (srcOff == dstoff)
	{
		uchar ch_t = *(src++) << srcOff;
		*(dst++) += (ch_t >> dstoff);
		/*if (srcOff)
		{
		dst++;
		src++;
		}*/
		while (dst < endDstWord)*(dst++) = *(src++);
		*dst = *src >> (8 - dstoff_t);
		*dst = *dst << (8 - dstoff_t);
		return;
	}
	uchar offset;
	uchar ch_t;
	if (srcOff > dstoff)
	{
		ch_t = *src << srcOff;
		*dst += (ch_t >> dstoff);
		offset = 8 - (srcOff - dstoff);
	}
	else
	{
		ch_t = *src << srcOff;
		*(dst++) += (ch_t >> dstoff);
		offset = dstoff - srcOff;
		*dst += (ch_t << (8 - dstoff));
	}
	while (dst < endDstWord)
	{
		uchar ch_t = *(++src);
		*(dst++) += (ch_t >> offset);
		*dst = (ch_t << (8 - offset));
	}
	ch_t = *(++src);
	*dst += (ch_t >> offset);
	*dst = *dst >> (8 - dstoff_t);
	*dst = *dst << (8 - dstoff_t);
}
void bitsCopy(uchar **dst, uchar *dstOff, uchar **src, uchar *srcOff, u32 len)
{
	uchar *dstPtr = *dst;
	uchar dstOff_t = *dstOff;
	uchar *srcPtr = *src;
	uchar srcOff_t = *srcOff;
	uchar *endDstWord = dstPtr + ((len + dstOff_t) >> 3);
	uchar dstoff_t = (len + dstOff_t) & 0x7;
	*dst = endDstWord;
	*dstOff = dstoff_t;
	*src = srcPtr + ((len + srcOff_t) >> 3);
	*srcOff = (len + srcOff_t) & 0x7;
	if (srcOff_t == dstOff_t)
	{
		uchar ch_t = *(srcPtr++) << srcOff_t;
		*(dstPtr++) += (ch_t >> dstOff_t);
		while (dstPtr < endDstWord)*(dstPtr++) = *(srcPtr++);
		*dstPtr = *srcPtr >> (8 - dstoff_t);
		*dstPtr = *dstPtr << (8 - dstoff_t);
		return;
	}
	uchar offset;
	uchar ch_t;
	if (srcOff_t > dstOff_t)
	{
		ch_t = *srcPtr << srcOff_t;
		*dstPtr += (ch_t >> dstOff_t);
		offset = 8 - (srcOff_t - dstOff_t);
	}
	else
	{
		ch_t = *srcPtr << srcOff_t;
		*(dstPtr++) += (ch_t >> dstOff_t);
		offset = dstOff_t - srcOff_t;
		*dstPtr += (ch_t << (8 - dstOff_t));
	}
	while (dstPtr < endDstWord)
	{
		uchar ch_t = *(++srcPtr);
		*(dstPtr++) += (ch_t >> offset);
		*dstPtr = (ch_t << (8 - offset));
	}
	ch_t = *(++srcPtr);
	*dstPtr += (ch_t >> offset);
	*dstPtr = *dstPtr >> (8 - dstoff_t);
	*dstPtr = *dstPtr << (8 - dstoff_t);
}
//下面是bitsCopy的测试函数
#if 0
int testBitsCopyIndex()
{
	int overNum = 500;
	int num = 1000000;
	uchar *dst = new uchar[num + overNum];
	uchar *src = new uchar[num + 10];
	int loopNum = 100;
	while (loopNum-- > 0)
	{

		memset(src, 0, num + 10);
		memset(dst, 0, num + overNum);
		for (int i = 0; i < num; i++)
		{
			src[i] = rand() % 200 + 32;
		}
		int sumLen = 0;
		srand(1484547112 + loopNum);
		u32 srcIndex = rand() % 7 + 1;
		u32 dstIndex = rand() % 7 + 1;
		u32 srcIndex_t = srcIndex;
		u32 dstIndex_t = dstIndex;
		while (sumLen < num * 8)
		{
			int len = rand() % 200 + 10;
			if (len < 10)cout << "len is error!" << endl;
			if (len + sumLen < num * 8)
			{
				bitsCopy(dst, dstIndex, src, srcIndex, len);
			}
			else
			{
				len = num * 8 - sumLen;
				bitsCopy(dst, dstIndex, src, srcIndex, len);
			}
			srcIndex += len;
			dstIndex += len;
			sumLen += len;
		}
		//验证
		for (int i = 0; i < num; i++)
		{
			uchar ch_ts = getOneUchar(src, srcIndex_t);
			uchar ch_td = getOneUchar(dst, dstIndex_t);
			if (ch_ts != ch_td)
			{
				cout << "copy failed!" << endl;
			}
			dstIndex_t += 8;
			srcIndex_t += 8;
		}
	}
	return 0;
}
int testCopyBits1()
{
	int overNum = 500;
	int num = 1000000;
	uchar *dst = new uchar[num + overNum];
	uchar *src = new uchar[num + 10];
	int loopNum = 100;
	while (loopNum-- > 0)
	{

		memset(src, 0, num + 10);
		memset(dst, 0, num + overNum);
		for (int i = 0; i < num; i++)
		{
			src[i] = rand() % 200 + 32;
		}
		int sumLen = 0;
		srand(1484547112 + loopNum);
		u32 srcIndex = rand() % 7 + 1;
		u32 dstIndex = rand() % 7 + 1;
		//u32 srcIndex = 0;
		//u32 dstIndex = 0;
		u32 srcIndex_t = srcIndex;
		u32 dstIndex_t = dstIndex;


		uchar *dst_t = dst;
		uchar *src_t = src;
		uchar dstOff = dstIndex & 0x7;
		uchar srcOff = srcIndex & 0x7;
		while (sumLen < num * 8)
		{
			int len = rand() % 200 + 10;
			if (len < 10)cout << "len is error!" << endl;
			if (len + sumLen < num * 8)
			{
				bitsCopy1(dst_t, dstOff, src_t, srcOff, len);
			}
			else
			{
				len = num * 8 - sumLen;
				bitsCopy1(dst_t, dstOff, src_t, srcOff, len);
			}
			sumLen += len;
			dst_t = dst_t + ((dstOff + len) >> 3);
			dstOff = (dstOff + len) & 0x7;
			src_t = src_t + ((srcOff + len) >> 3);
			srcOff = (srcOff + len) & 0x7;
		}
		//验证
		for (int i = 0; i < num; i++)
		{
			uchar ch_ts = getOneUchar(src, srcIndex_t);
			uchar ch_td = getOneUchar(dst, dstIndex_t);
			if (ch_ts != ch_td)
			{
				cout << "copy failed!" << endl;
			}
			dstIndex_t += 8;
			srcIndex_t += 8;
		}
	}
	delete[] src;
	delete[] dst;
	return 0;
}
int testCopyBits()
{
	int overNum = 500;
	int num = 1000000;
	uchar *dst = new uchar[num + overNum];
	uchar *src = new uchar[num + 10];
	int loopNum = 100;
	while (loopNum-- > 0)
	{

		memset(src, 0, num + 10);
		memset(dst, 0, num + overNum);
		for (int i = 0; i < num; i++)
		{
			src[i] = rand() % 200 + 32;
		}
		int sumLen = 0;
		srand(1484547112 + loopNum);
		u32 srcIndex = rand() % 7 + 1;
		u32 dstIndex = rand() % 7 + 1;
		u32 srcIndex_t = srcIndex;
		u32 dstIndex_t = dstIndex;


		uchar *dst_t = dst;
		uchar *src_t = src;
		uchar dstOff = dstIndex & 0x7;
		uchar srcOff = srcIndex & 0x7;
		while (sumLen < num * 8)
		{
			int len = rand() % 200 + 10;
			if (len < 10)cout << "len is error!" << endl;
			if (len + sumLen < num * 8)
			{
				bitsCopy(&dst_t, &dstOff, &src_t, &srcOff, len);
			}
			else
			{
				len = num * 8 - sumLen;
				bitsCopy(&dst_t, &dstOff, &src_t, &srcOff, len);
			}
			sumLen += len;
		}
		//验证
		for (int i = 0; i < num; i++)
		{
			uchar ch_ts = getOneUchar(src, srcIndex_t);
			uchar ch_td = getOneUchar(dst, dstIndex_t);
			if (ch_ts != ch_td)
			{
				cout << "copy failed!" << endl;
			}
			dstIndex_t += 8;
			srcIndex_t += 8;
		}
	}
	delete[] src;
	delete[] dst;
	return 0;
}
int main()
{
	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nBeginTime);//开始计时 

	testBitsCopyIndex();

	QueryPerformanceCounter(&nEndTime);//停止计时 
	time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time1 * 1000 << "ms" << endl;


	QueryPerformanceCounter(&nBeginTime);//开始计时 

	testCopyBits1();

	QueryPerformanceCounter(&nEndTime);//停止计时 
	time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time1 * 1000 << "ms" << endl;


	QueryPerformanceCounter(&nBeginTime);//开始计时 

	testCopyBits();

	QueryPerformanceCounter(&nEndTime);//停止计时 
	time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time1 * 1000 << "ms" << endl;
	return 0;
}
#endif 




uchar getMark2(uchar **src, uchar *offset)
{
	uchar *ptr = *src;
	uchar offset_t = *offset;
	if ((offset_t + 2) & 0x8)
	{//>8
		u16 val_t = *(ptr++);
		val_t = (val_t << 8) + *ptr;
		val_t = val_t << offset_t;
		*offset = (*offset + 2) & 0x7;
		*src = ptr;
		return (val_t >> (14)) & 0xff;
	}
	uchar ch_t = *ptr << offset_t;
	*offset += 2;
	*src = ptr;
	return ch_t >> 6;
}
void writeMark2(uchar **src, uchar *offset, uchar val)
{
	uchar *ptr = *src;
	uchar offset_t = *offset;
	if ((offset_t + 2) & 0x8)
	{
		u16 val_t = *ptr;
		val_t = (val_t << (offset_t - 6)) + val;
		val_t = val_t << (14 - offset_t);
		*(ptr++) = val_t >> 8;
		*ptr = val_t & 0xff;
		*offset = (*offset + 2) & 0x7;
		*src = ptr;
		return;
	}
	*ptr += val << (6 - offset_t);
	*src = ptr;
	*offset += 2;
}
uchar getMark(uchar **src, uchar *offset, uchar len)
{
	uchar *ptr = *src;
	uchar offset_t = *offset;
	if ((offset_t + len) & 0x8)
	{//>8
		u16 val_t = *(ptr++);
		val_t = (val_t << 8) + *ptr;
		val_t = val_t << offset_t;
		*offset = (*offset + len) & 0x7;
		*src = ptr;
		return (val_t >> (16 - len)) & 0xff;
	}
	uchar ch_t = *ptr << offset_t;
	*offset += len;
	*src = ptr;
	return ch_t >> (8 - len);
}
void writeMark(uchar **src, uchar *offset, uchar val, uchar len)
{
	uchar *ptr = *src;
	uchar offset_t = *offset;
	if ((offset_t + len) & 0x8)
	{
		u16 val_t = *ptr;
		val_t = (val_t << (len + offset_t - 8)) + val;
		val_t = val_t << (16 - len - offset_t);
		*(ptr++) = val_t >> 8;
		*ptr = val_t & 0xff;
		*offset = (*offset + len) & 0x7;
		*src = ptr;
		return;
	}
	*ptr += val << (8 - offset_t - len);
	*src = ptr;
	*offset += len;
}
//下面是getMark与writeMark的测试函数
#if 0
int main()
{

	int num = 10000;
	uchar *dst = new uchar[num];
	memset(dst, 0, num);
	uchar *head = new uchar[num * 5];
	uchar *headLen = new uchar[num * 5];
	u32 distLen = 0;

	uchar offset = 3;
	uchar*dst_t = dst;
	uchar offset_t = offset;
	int i = -1;
	while (i < num)
	{
		head[++i] = rand() % 256 + 1;
		//head[++i] = 5;
		headLen[i] = getBitsOfNum(head[i]) + 1;
		writeMark(&dst_t, &offset_t, head[i], headLen[i]);
		distLen += headLen[i];
	}
	distLen = 0;
	dst_t = dst;
	offset_t = offset;
	i = 0;
	while (i <= num)
	{
		uchar ch_t = getMark(&dst_t, &offset_t, headLen[i]);
		distLen += headLen[i];
		if (ch_t != head[i++])
		{
			cout << "nmu=" << num << "  " << dst_t - dst << endl;
			cout << "error!" << endl;
		}
	}
	delete[] dst;
	return 0;
}
#endif




uchar getNextOneBit(uchar *src, uchar offset)
{
	return src[0] & (0x80 >> offset);
}
uchar getOneBits(uchar *src, u32 index)
{
	u32 words = index >> 3;
	uchar offset = index & 0x7;
	return src[words] & (128 >> offset);
}
//获取一个比特函数的测试程序
#if 0
int main()
{
	u32 words = 0;
	uchar offset = 0;
	int num = 100000;
	uchar *src = new uchar[num];
	int srcIndex = 0;
	for (int i = 0; i < num; i++)
	{
		src[i] = rand() % 256;
	}
	for (; words < num;)
	{
		uchar ch_t1 = getNextOneBit(&src[words], offset);
		uchar ch_t2 = getOneBits(src, srcIndex);
		if (ch_t1 != ch_t2)
		{
			cout << "error!" << ch_t1 << " " << ch_t2 << endl;
		}
		offset++;
		if (offset & 0x8)
		{
			offset = offset & 0x7;
			words++;
		}
		srcIndex++;
	}

	return 0;
}
#endif 





uchar acRunsTbl[256] = {//run length gamma
	8,
	7,
	6, 6,
	5, 5, 5, 5,
	4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5,
	6, 6,
	7,
	8
};
uchar *EndWords = NULL;
uchar EndOff=0;
u32 getRuns(uchar **src, uchar *offset)
{
	uchar *ptr = *src;
	uchar off_t = *offset;
	uchar ch_t = *ptr << off_t;
	//u32 runs = acRunsTbl[(*ptr)<<off_t];
	u32 runs = acRunsTbl[ch_t];
	if (runs == 0)
	{
		int xxx = 0;

		runs = acRunsTbl[ch_t];
	}
	if (runs < 8 - off_t)
	{
		*offset += runs;
		return runs;
	}
	runs = 8 - off_t;
	while (++ptr < EndWords){
		uchar ch_t1 = *ptr >> 7;
		uchar ch_t2 = *(ptr - 1) & 1;
		if ((*ptr >> 7) == ((*(ptr - 1)) & 1))
		{
			uchar runs_t = acRunsTbl[*ptr];
			runs += runs_t;
			if (runs_t < 8)
			{
				*src = ptr;
				*offset = runs_t;
				return runs;
			}
		}
		else{
			*src = ptr;
			*offset = 0;
			return runs;
		}
	}
	
	runs = (EndWords - *src) * 8 + EndOff - *offset;
	*src = EndWords;
	*offset = EndOff;
	/*if((*(ptr-1))&(128>>EndOff))
	{
		*offset=(runs+off_t)&0x7;
		return runs;
	}
	runs=runs-(8-EndOff);
	*offset=(runs+off_t)&0x7;*/
	return EndOff?runs-8:runs;
}
u32 runLengthCode(uchar *src, u32 bitsLen, uchar *dst, u16 *Runs)
{
	int k = 0;

	int ret = 0;
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
			Runs[k++] = period;
			//return period;
			if (ret<0)
			{
				return ret;
			}
			period = 1;//reset the length of run
			flag = flag ? false : true;// switch the mark of runs
		}

	}

	//return period;
	Runs[k++] = period;
	return (dst - savedDst) * 8 + offset;
}
void writeRuns(uchar **src, uchar *offset, u32 runs)
{
	//uchar mask = 255;
	uchar *ptr = *src;
	uchar off = *offset;
	uchar *src_t = ptr + ((off + runs) >> 3);
	uchar src_off = (off + runs) & 0x7;
	*src = src_t;
	*offset = src_off;
	*ptr += 0xff >> off;
	while (ptr < src_t)
	{
		*(++ptr) = 0xff;
	}
	*ptr = *ptr&(0xff << (8 - src_off));
}
void writeRuns_t(uchar *src, uchar offset, u32 runs)
{
	//uchar mask = 255;
	uchar *src_t = src + ((offset + runs) >> 3);
	uchar src_off = (offset + runs) & 0x7;
	*src += 0xff >> offset;
	while (src < src_t)
	{
		*(++src) = 0xff;
	}
	*src = *src&(0xff << (8 - src_off));
}
//getRuns的测试程序，还有时间加速测试
#if 0
int main()
{
	//测试程序运行时间
	double time = 0;
	double counts = 0;
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nBeginTime;
	LARGE_INTEGER nEndTime;
	QueryPerformanceFrequency(&nFreq);

	u32 num = 1000000;
	u32 bitsLen = num * 8;
	uchar *src = new uchar[num];
	uchar srcOff = 0;
	uchar *srcPtr_t = src;
	uchar srcOff_t = srcOff;
	EndWords = src + num;
	srand(num);
	for (int i = 0; i < num; i++)
	{
		int tmp1 = rand() % 2;
		switch (tmp1)
		{
		case 0:
			src[i] = rand() % 16;
			break;
		case 1:
			src[i] = rand() % 16 + 240;
			break;
		default:
			break;
		}
	}

	u16 *Runs = new u16[num * 5];
	for (int i = 0; i < num * 5; i++)
	{
		Runs[i] = 0;
	}
	int k = 0;
	QueryPerformanceCounter(&nBeginTime);//开始计时  
	while (srcPtr_t != EndWords)
	{
		//u32 getRuns(uchar **src, uchar *offset)
		if (k >= 53323)
		{
			int xxx = 0;
		}
		Runs[k++] = getRuns(&srcPtr_t, &srcOff_t);
	}
	QueryPerformanceCounter(&nEndTime);//停止计时 
	time = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time * 1000 << "ms" << endl;
	u16 *Runs1 = new u16[num * 5];
	for (int i = 0; i < num * 5; i++)
	{
		Runs1[i] = 0;
	}
	uchar *dst = new uchar[10];
	QueryPerformanceCounter(&nBeginTime);//开始计时 
	runLengthCode(src, num * 8, dst, Runs1);
	QueryPerformanceCounter(&nEndTime);//停止计时 
	time = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time * 1000 << "ms" << endl;
	for (int i = 0; i < num * 5; i++)
	{
		if (Runs[i] != Runs1[i])
		{
			cout << i << "\t" << Runs[i] << "\t" << Runs1[i] << " Runs error!" << endl;
		}
	}

	uchar *runsTsrc = new uchar[num];
	uchar *rTs_t = runsTsrc;
	uchar  rTr_off = 0;
	memset(runsTsrc, 0, num);
	bool flag = src[0] & 128;
	for (int i = 0; i < num * 5; i++)
	{
		if (Runs[i] < 1)
		{
			continue;
		}
		if (flag)
		{
			writeRuns(&runsTsrc, &rTr_off, Runs[i]);
			flag = !flag;
			continue;
		}
		runsTsrc += (rTr_off + Runs[i]) >> 3;
		rTr_off = (rTr_off + Runs[i]) & 0x7;
		flag = !flag;
	}

	for (int i = 0; i < num; i++)
	{
		if (src[i] != rTs_t[i])
		{
			cout << (u32)src[i] << " " << (int)rTs_t[i] << "\terror!" << endl;
		}
	}

	return 0;
}
#endif




int DiffGPTbls[512] = {//gamma-plusOne
	-1, -1, 1, -2, 0, 0, 0, -1, 1, 1, 1, 1, 1, 1, 1, -2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};
int getSubGP(u32 x)
{
	if (x<512){
		return DiffGPTbls[x];
	}
	else
	{
		int numGa = (getBitsOfNum(x) << 1) + 1;
		int numDe = (getBitsOfNum(getBitsOfNum(x + 1)) << 1) + getBitsOfNum(x + 1) + 1;
		return numGa - numDe;
	}
}
//getSubGP的測試程序如下
#if 0
int main()
{
	for (u32 i = 1; i < 4096; i++)
	{
		int numGa = (getBitsOfNum(i) << 1) + 1;
		int numDe = (getBitsOfNum(getBitsOfNum(i + 1)) << 1) + getBitsOfNum(i + 1) + 1;
		if (numGa - numDe != getSubGP(i))
		{
			cout << "error!" << endl;
		}
	}
	cout << endl;
	double time = 0;
	double counts = 0;
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nBeginTime;
	LARGE_INTEGER nEndTime;
	int gapBits = 0;
	QueryPerformanceFrequency(&nFreq);
	int k;
	QueryPerformanceCounter(&nBeginTime);//开始计时  
	k = 0;
	while (k < 20)
	{
		for (u32 i = 1; i < 1024; i++)
		{
			int numGa = (getBitsOfNum(i) << 1) + 1;
			int numDe = (getBitsOfNum(getBitsOfNum(i + 1)) << 1) + getBitsOfNum(i + 1) + 1;
			gapBits += numGa - numDe;
		}
		k++;

	}
	QueryPerformanceCounter(&nEndTime);//停止计时 
	time = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time * 1000 << "ms" << endl;
	QueryPerformanceCounter(&nBeginTime);//开始计时  
	k = 0;
	while (k < 20)
	{
		for (u32 i = 1; i < 1024; i++)
		{
			gapBits += getSubGP(i);
		}
		k++;

	}
	QueryPerformanceCounter(&nEndTime);//停止计时 
	time = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time * 1000 << "ms" << endl;
	return 0;
}
#endif 

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
#if 0
//写入Runs的测试程序

int main()
{
	int Nums = 1000000;
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
	for (int i = 0; i < Rnums; i++)
	{
		Runs[i] = rand() % 256 + 1;
	}
	//存储1
	/*for (int i = 0; i < Rnums; i++)
	{
	elisGammaCode1(Runs[i], &dst_t, &dOff);
	Append_g(Runs[i], &dst1_t, &d1Off);
	}*/
	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nBeginTime);//开始计时 
	for (int i = 0; i < Rnums; i++)
	{
		elisGammaCode1(Runs[i], &dst_t, &dOff);
	}
	QueryPerformanceCounter(&nEndTime);//停止计时 
	time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time1 * 1000 << "ms" << endl;

	QueryPerformanceCounter(&nBeginTime);//开始计时 
	for (int i = 0; i < Rnums; i++)
	{
		Append_g(Runs[i], &dst1_t, &d1Off);
	}
	QueryPerformanceCounter(&nEndTime);//停止计时 
	time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
	cout << "程序执行时间：" << time1 * 1000 << "ms" << endl;
	int k = 0;
	while (dst[k] == dst1[k] && k < Nums)
	{
		k++;
	}
	cout << k << endl;
	delete[]Runs;
	delete[]dst;
	delete[]dst1;
	return 0;
}
#endif
#if 0
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
#endif

u16 plusOlen[512] = {
	4864, 4352, 3840, 3840, 3328, 3328, 3328, 3328,
	2816, 2816, 2816, 2816, 2816, 2816, 2816, 2816,
	2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327,
	2328, 2329, 2330, 2331, 2332, 2333, 2334, 2335,
	1800, 1800, 1800, 1800, 1801, 1801, 1801, 1801,
	1802, 1802, 1802, 1802, 1803, 1803, 1803, 1803,
	1804, 1804, 1804, 1804, 1805, 1805, 1805, 1805,
	1806, 1806, 1806, 1806, 1807, 1807, 1807, 1807,
	2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359,
	2360, 2361, 2362, 2363, 2364, 2365, 2366, 2367,
	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,
	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,
	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,
	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,
	1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287,
	1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287,
	1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316,
	1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316,
	1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317,
	1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317,
	1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318,
	1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318,
	1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319,
	1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319,
	1576, 1576, 1576, 1576, 1576, 1576, 1576, 1576,
	1577, 1577, 1577, 1577, 1577, 1577, 1577, 1577,
	1578, 1578, 1578, 1578, 1578, 1578, 1578, 1578,
	1579, 1579, 1579, 1579, 1579, 1579, 1579, 1579,
	1580, 1580, 1580, 1580, 1580, 1580, 1580, 1580,
	1581, 1581, 1581, 1581, 1581, 1581, 1581, 1581,
	1582, 1582, 1582, 1582, 1582, 1582, 1582, 1582,
	1583, 1583, 1583, 1583, 1583, 1583, 1583, 1583,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546,	546, 546, 546, 546, 546, 546, 546, 546,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547,	547, 547, 547, 547, 547, 547, 547, 547
};
u16 getBitsPO(uchar *src, uchar srcOff)
{
	u16 val = src[0];
	val = (val << 8) + src[1];
	return val << srcOff;
}
u32 getBitsPO1(uchar *src, uchar srcOff, u16 len)
{
	u32 val = *src++;
	val = (val << 8) + *src++;
	if (len < 16 - srcOff)
	{
		val = val << (16 + srcOff);
		return val >> (32 - len);
	}
	val = (val << 8) + *src++;
	if (len < 24 - srcOff)
	{
		val = val << (8 + srcOff);
		return val >> (32 - len);
	}
	val = (val << 8) + *src++;
	if (len < 32 - srcOff)
	{
		val = val << srcOff;
		return val >> (32 - len);
	}
}
#if 0
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
	u16 *tabl_t = new u16[512];
	memset(tabl_t, 0, 512*sizeof(u16));
	for(u16 i=0;i<512;i++)
	{
		R1 = R2 = 0;
		u16 zeros = findZeroU16_9(i);
		u16 gLen = zeros * 2 + 1;
		u16 gamma = i >> (9 - gLen);
		u16 plusOne = 0;
		R1 = gamma;
		R2 = gLen;
		if (gamma < 5&&gamma>0)
		{
			plusOne = 1 << gamma;
			u16 tmp = i << (7 + gLen);
			plusOne += tmp >> (16 - gamma);
			R1 = plusOne + 32;
			R2 += gamma;
		}
		tabl_t[i] = R2 << 8;
		tabl_t[i] += R1;
		cout << i << "\t";
		printUchar_t(i>>8);
		printUchar_t(i&0xff);
		cout<<(int)findZeroU16_9(i)<<" "<<gamma<<"  "<<plusOne<<endl;
	}
	
	ofstream out;
	out.open("plusOne.txt", ios::out);
	if (!out.is_open())
	{
		cout << "open file failed!" << endl;
	}

	for (u16 i = 0; i <512; i++)
	{
		uchar R1, R2;
		R1 = tabl_t[i] & 0xff;
		R2 = tabl_t[i] >> 8;
		cout << i << "\t";
		printUchar_t(i >> 8);
		printUchar_t(i & 0xff);
		if (i % 8 == 0)
		{
			cout << endl;
			out << "\n";
		}
		int tmp = tabl_t[i];
		out << tmp << ",";
		cout << "\t"<<tabl_t[i]<<"\t" <<(int)R1 << "\t" <<(int)R2 << endl;
	}
	out.close();
	return 0;
}

#endif
u16 degaTab[512] = {
	2304, 2048, 1792, 1792, 1536, 1536, 1536, 1536, 1280, 1280, 1280, 1280, 1280, 1280, 1280, 1280,
	2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 2328, 2329, 2330, 2331, 2332, 2333, 2334, 2335,
	1800, 1800, 1800, 1800, 1801, 1801, 1801, 1801, 1802, 1802, 1802, 1802, 1803, 1803, 1803, 1803,
	1804, 1804, 1804, 1804, 1805, 1805, 1805, 1805, 1806, 1806, 1806, 1806, 1807, 1807, 1807, 1807,
	1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284,
	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,
	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,
	1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257
};

uchar getRunsForDeg(uchar **src, uchar *srcOff)
{
	uchar Runs = 8;
	uchar *src_t = *src + 1;
	while (*src_t == 0)
	{
		src_t++;
		Runs += 8;
	}
	uchar srcOff_t = *srcOff;
	*srcOff = (*src_t) & 128 ? 0 : acRunsTbl[*src_t];
	*src = src_t;
	return Runs + *srcOff - srcOff_t;
}
int bugs_t=0;
void deCodeGamma_224(bool *flag, uchar **src, uchar *srcOff,
	uchar **dst, uchar *dstOff, u32 HBblSize)
{
	uchar *dst_t = *dst;
	uchar dstOff_t = *dstOff;
	uchar *src_t = *src;
	uchar srcOff_t = *srcOff;
	u16 val;
	uchar X9 = 0x1ff;
	uchar R1, R2, Rt;
	u32 Runs;
	u32 deLen = 0;
	bool flag_t = *flag;
	while (src_t < EndWords)
	{
		val = getBitsPO(src_t, srcOff_t);
		u16 val_t = val >> 7;
		R2 = degaTab[val_t] >> 8;
		Runs = degaTab[val_t] & 0xff;
		if (Runs)
		{
			if (flag_t)
				writeRuns_t(dst_t, dstOff_t, Runs);
		}
		else{
			if (R2 < 9){
				src_t += (srcOff_t + R2) >> 3;
				srcOff_t = (srcOff_t + R2) & 0x7;
				Runs = getBitsPO1(src_t, srcOff_t, R2 + 1);
			}
			else{
				R2 = getRunsForDeg(&src_t, &srcOff_t);
				Runs = getBitsPO1(src_t, srcOff_t, R2 + 1);
			}
			if (flag_t)
				writeRuns_t(dst_t, dstOff_t, Runs);
			R2 = (R2 << 1) + 1;
		}
		if (Runs == 2)
		{
			bugs_t = 1;
		}
		else if (Runs == 13 && bugs_t)
		{
			bugs_t = 2;
		}
		else
		{
			bugs_t = 0;
		}
		val_t = val_t << R2;
		dst_t += (dstOff_t + Runs) >> 3;
		dstOff_t = (dstOff_t + Runs) & 0x7;
		deLen += Runs;
		while ((val_t & 256) && HBblSize > deLen){
			flag_t = !flag_t;
			if (flag_t)
				(*dst_t) += 128 >> dstOff_t;
			dstOff_t++;
			if (dstOff_t == 8){
				dst_t++;
				dstOff_t = 0;
			}
			R2++;
			deLen++;
			val_t = val_t << 1;
		}
		src_t += (srcOff_t + R2) >> 3;
		srcOff_t = (srcOff_t + R2) & 0x7;
		flag_t = !flag_t;
		if (HBblSize <= deLen)
		{
			*flag = flag_t;
			*src = src_t;
			*srcOff =srcOff_t;
			*dst = dst_t;
			*dstOff = dstOff_t;
			return;
		}
		
	}
	while (srcOff_t < EndOff)
	{
		val = getBitsPO(src_t, srcOff_t);
		u16 val_t = val >> 7;
		R2 = degaTab[val_t] >> 8;
		Runs = degaTab[val_t] & 0xff;
		if (flag_t)
			writeRuns_t(dst_t, dstOff_t, Runs);
		flag_t = !flag_t;
		src_t = src_t + ((R2 + srcOff_t) >> 3);//可以删除
		srcOff_t = (R2 + srcOff_t) & 0x7;
		if (HBblSize <= deLen)
		{
			*flag = flag_t;
			*src = src_t;
			*srcOff = srcOff_t;
			return;
		}
	}
	*src = EndWords + 1;
}
int runLengthGammaDecode_228(uchar *src, u32 bitsLen, uchar *dst)
{
	uchar srcOffset;
	uchar dstOffset;

	uchar *savedSrc = src;
	uchar *savedDst = dst;

	//*dst = src[0] & 128;
	bool flag = src[0] ? true : false;

	srcOffset = 1;
	dstOffset = 0;

	u32 Runs, deLen = 1;
	uchar R1, R2, Rt;
	while (deLen < bitsLen)
	{
		u16 val = *src;
		val = (val << 8) | src[1];
		val = val << srcOffset;
		u16 val_t = val >> 7;
		R2 = degaTab[val_t] >> 8;
		Runs = degaTab[val_t] & 0xff;
		deLen += R2;
		if (Runs)
		{
			if (flag)
				writeRuns_t(dst, dstOffset, Runs);
		}
		else
		{
			
			
			if (R2 < 9){
				src += (srcOffset + R2) >> 3;
				srcOffset = (srcOffset + R2) & 0x7;
				R2++;
				Runs = getBitsPO1(src, srcOffset, R2);
			}
			else{
				R2 = getRunsForDeg(&src, &srcOffset);
				R2++;
				Runs = getBitsPO1(src, srcOffset, R2);
			}
			if (flag)
				writeRuns_t(dst, dstOffset, Runs);
			deLen += R2;
		}
		flag = !flag;
		dst += (dstOffset + Runs) >> 3;
		dstOffset = (dstOffset + Runs) & 0x7;
		src += (srcOffset + R2) >> 3;
		srcOffset = (srcOffset + R2) & 0x7;
		
	}
	return (dst - savedDst) * 8 + dstOffset;
}
#if 0
//测试新的gamma加速表
int main()
{
}
#endif



