#include"BitsCopyFunc.h"

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
//传值，调用该函数是变量不会改变
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
		uchar ch_t = *srcPtr << srcOff_t;
		*dstPtr += (ch_t >> dstOff_t);
		/*if (srcOff_t){
		dstPtr++;
		srcPtr++;}*/
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
#if 1

#endif 
#if 0
int main()
{
	int overNum = 500;
	int num = 1000000;

	uchar *src = new uchar[num];
	uchar *dst = new uchar[num + overNum];
	uchar *dst_t = dst;
	uchar *src_t = src;
	uchar srcOff = 0;
	uchar dstOff = 0;
	memset(dst, 0, num + overNum);
	for (int i = 0; i < num; i++)
	{
		src[i] = rand() % 200 + 32;
	}
	int sumLen = 0;
	while (sumLen < num * 8)
	{
		u32 len = rand() % 200 + 10;
		if (len < 10)cout << "len is error!" << endl;
		if (len + sumLen < num * 8)
		{
			bitsCopy(dst_t, dstOff, src_t, srcOff, len);
		}
		else
		{
			len = num * 8 - sumLen;
			bitsCopy(dst_t, dstOff, src_t, srcOff, len);
		}
		dst_t += (dstOff + len) >> 3;
		dstOff = (dstOff + len) & 0x7;
		src_t += (srcOff + len) >> 3;
		srcOff = (srcOff + len) & 0x7;
		sumLen += len;
	}
	//验证
	for (int i = 0; i < num; i++)
	{
		if (src[i] != dst[i])
		{
			cout << "copy failed!" << endl;
		}
	}
	return 0;
}
#endif