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
	*dst = *src&(0x80);//����һ��bitд��ѹ����
	bool flag = (bool)getNextOneBit(src, 0);//inmportant
	uchar dstOffset = 1;
	uchar srcOffset = 0;
	//��¼��һ�������λ��
	uchar *lastSrcWord;
	uchar lastSrcOff;
	//Runs����
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
			writeMark2(&dst, &dstOffset,3);//д����뷽ʽ
			//д��PlusOne
			continue;
		}
		for (int i = 0; i < k; i++)//����k-1���������Կ��С
			sumBits += getBitsOfNum(Runs[i]);
		sumBits = (sumBits << 1) + k;
		if (src != EndWords)
		{
			if (sumBits < HBblSize)
			{
				if (gapBits < 0)
				{
					writeMark2(&dst, &dstOffset, 4);//д����뷽ʽ
					//д��Gamma
					continue;
				}
				writeMark2(&dst, &dstOffset, 3);//д����뷽ʽ
				//д��PlusOne
				continue;
			}
			//����flag
			if (sumBits > HBblSize)
			{
				flag = !flag;//����runs
				u32 bits_t = sumBits - HBblSize;
				
			}	
			if(flag)
				writeMark2(&dst, &dstOffset, 1);//д����뷽ʽ
			else
				writeMark2(&dst, &dstOffset, 0);//д����뷽ʽ
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
				writeMark2(&dst, &dstOffset, 4);//д����뷽ʽ
				//д��Gamma
				continue;
			}
			writeMark2(&dst, &dstOffset, 3);//д����뷽ʽ
			//д��PlusOne
			continue;

		}
		else{//�������Ӧ�ü��ٳ���
			cout << "����������֣�" << endl;
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