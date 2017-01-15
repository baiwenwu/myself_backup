#include"testFunction.h"

double time1 = 0;
double counts = 0;
LARGE_INTEGER nFreq;
LARGE_INTEGER nBeginTime;
LARGE_INTEGER nEndTime;

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

#if 0
void Append_g(uchar*cdata, u32 index, u32 runs)
{//runlength gamma
	u32 zerosNum = getBitsOfNum(runs);
	index += zerosNum;
	u32 wordsL = index >> 3;
	index += zerosNum + 1;
	u32 wordsR = index >> 3;
	u32 offset = index & 0x7;
	if (!offset)
	{
		offset = 8;
		wordsR--;
	}
	runs = runs << (8 - offset);
	while (wordsR > wordsL)
	{
		cdata[wordsR] += runs & 0xff;
		runs = runs >> 8;
		wordsR--;
	}
	cdata[wordsR] += runs & 0xff;
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
	runs = runs << (8-off);
	*ptr += runs & 0xff;
	if (ptr == ptrs)
		return;
	*(--ptr) += (runs>>8) & 0xff;
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

#if 1
int main()
{
	int Nums = 1000000;
	int Rnums = Nums >> 1;
	u32 *Runs = new u32[Rnums];
	//�洢�ṹ
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
	//�洢1
	/*for (int i = 0; i < Rnums; i++)
	{
		elisGammaCode1(Runs[i], &dst_t, &dOff);
		Append_g(Runs[i], &dst1_t, &d1Off);
	}*/
	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nBeginTime);//��ʼ��ʱ 
	for (int i = 0; i < Rnums; i++)
	{
		elisGammaCode1(Runs[i], &dst_t, &dOff);
	}
	QueryPerformanceCounter(&nEndTime);//ֹͣ��ʱ 
	time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//�������ִ��ʱ�䵥λΪs 
	cout << "����ִ��ʱ�䣺" << time1 * 1000 << "ms" << endl;

	QueryPerformanceCounter(&nBeginTime);//��ʼ��ʱ 
	for (int i = 0; i < Rnums; i++)
	{
		Append_g(Runs[i], &dst1_t, &d1Off);
	}
	QueryPerformanceCounter(&nEndTime);//ֹͣ��ʱ 
	time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//�������ִ��ʱ�䵥λΪs 
	cout << "����ִ��ʱ�䣺" << time1 * 1000 << "ms" << endl;
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