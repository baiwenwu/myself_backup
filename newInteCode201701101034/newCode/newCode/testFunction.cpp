#include"testFunction.h"

int gppHybirdCode(uchar *src, u32 bitsLen, uchar *dst, u16 HBblSize)
{
	int ret;
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}
	uchar *savedDst = dst;
	//uchar *savedSrc = src;
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
			rbits += Runs[k++];
			gapBits = getSubGP(Runs[k++]);
			flag = !flag;
		}
		if (k == 1 && HBblSize<rbits)
		{//plusOne is good
			writeMark2(&dst, &dstOffset,3);//写入编码方式
			//写入PlusOne
			continue;
		}
		for (int i = 0; i < k; i++)//可以k-1，用来调试块大小
			sumBits += getBitsOfNum(Runs[i]);
		sumBits = (sumBits << 1) + k;
		if (src != EndWords)
		{
			if (sumBits < HBblSize)
			{
				if (gapBits < 0)
				{
					writeMark2(&dst, &dstOffset, 4);//写入编码方式
					//写入Gamma
					continue;
				}
				writeMark2(&dst, &dstOffset, 3);//写入编码方式
				//写入PlusOne
				continue;
			}
			//调整flag
			if (sumBits > HBblSize)
			{
				flag = !flag;//调整runs
				u32 bits_t = sumBits - HBblSize;
				
			}	
			if(flag)
				writeMark2(&dst, &dstOffset, 1);//写入编码方式
			else
				writeMark2(&dst, &dstOffset, 0);//写入编码方式
			//copy();
			//bitsCopy(dst, dstOffset, src, srcOffset, srcOffset, HBblSize);
			src = lastSrcWord + (HBblSize >> 3);
			srcOffset = lastSrcOff;
			continue;
		}
		if (sumBits <= rbits)
		{
			if (gapBits <= 0)
			{
				writeMark2(&dst, &dstOffset, 4);//写入编码方式
				//写入Gamma
				continue;
			}
			writeMark2(&dst, &dstOffset, 3);//写入编码方式
			//写入PlusOne
			continue;

		}
		else{//这种情况应该极少出现
			cout << "极少情况出现！" << endl;
		}
	}
	return (dst - savedDst) * 8 + dstOffset;
}

#if 1
int main()
{
	return 0;
}
#endif