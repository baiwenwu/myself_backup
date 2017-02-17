#include"oneplus.h"
#include<stdlib.h>
int bitsNumTbl[256] = {
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
int getBitsNum(unsigned x)
{
	if (x<256){
		return bitsNumTbl[x];
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




int elisGammaCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	int ret;
	if (!num || !buffPPtr || !offset)
	{
		return -1;
	}
	uchar *ptr = *buffPPtr;
	uchar off = *offset;

	u32 bitsLen = getBitsNum(num);
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
int elisGammaDecode(u32 *num, uchar **buffPPtr, uchar* offset)
{
	int bisLen = 0;
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	while (!(*ptr&(1 << (7 - off))))
	{
		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
		bisLen++;
	}

	unsigned sum = 0;
	int i;

	for (i = 0; i <= bisLen; i++)
	{
		sum = (sum << 1) + ((*ptr&(1 << (7 - off))) ? 1 : 0);
		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
	}

	*buffPPtr = ptr;
	*offset = off;
	*num = sum;
	return 0;
}
int elisDeltaCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	if (!num || !buffPPtr || !offset)
	{
		return -1;
	}

	u32 bitsLen = getBitsNum(num);
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	elisGammaCode(bitsLen + 1, &ptr, &off);

	u32 i;
	for (i = 1; i <= bitsLen; i++)
	{
		if (num &(1 << (bitsLen - i)))
		{
			*ptr |= 1 << (7 - off);
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
int elisDeltaDecode(u32 *num, uchar **buffPPtr, uchar* offset)
{
	u32 i;
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	elisGammaDecode(&i, &ptr, &off);

	u32 bitsLen = i - 1;
	u32 sum = 1;
	for (i = 0; i<bitsLen; i++)
	{
		sum = (sum << 1) + (((*ptr)&(1 << (7 - off)))
			>>
			(7 - off)
			);
		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
	}

	*buffPPtr = ptr;
	*offset = off;
	*num = sum;
	return 0;
}
int plusOneCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	if (!num || !buffPPtr || !offset)
	{
		return -1;
	}
	num = num + 1;
	u32 bitsLen = getBitsNum(num);
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	elisGammaCode(bitsLen, &ptr, &off);

	u32 i;
	for (i = 1; i <= bitsLen; i++)
	{
		if (num &(1 << (bitsLen - i)))
		{
			*ptr |= 1 << (7 - off);
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
int plusOneDecode(u32 *num, uchar **buffPPtr, uchar* offset)
{
	u32 i;
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	elisGammaDecode(&i, &ptr, &off);

	u32 bitsLen = i;
	u32 sum = 1;
	for (i = 0; i<bitsLen; i++)
	{
		sum = (sum << 1) + (((*ptr)&(1 << (7 - off)))
			>>
			(7 - off)
			);
		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
	}

	*buffPPtr = ptr;
	*offset = off;
	*num = sum-1;
	return 0;
}
int runLengthPlusOneCode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}

	uchar *savedDst = dst;
	uchar dstOffset = 0;

	//get the first bit
	*dst = *src;
	dstOffset++;

	bool flag;
	if (*src&(0x1 << 7))
	{
		flag = true;
	}
	else
	{
		flag = false;
	}

	u32 i, period;
	bool flag1, flag2;

	period = 1;
	for (i = 1; i<bitsLen; i++)
	{
		flag1 = (src[i / 8] & (1 << (7 - i % 8))) && flag;
		flag2 = (src[i / 8] & (1 << (7 - i % 8))) || flag;

		if (flag1 || !flag2)
		{
			period++;
		}
		else
		{
			plusOneCode(period, &dst, &dstOffset);
			period = 1;//reset the length of runs
			flag = !flag;//switch the marker of the runs
		}
	}

	plusOneCode(period, &dst, &dstOffset);

	return (dst - savedDst) * 8 + dstOffset;
}
int runLengthDeltaCode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}

	uchar *savedDst = dst;
	uchar dstOffset = 0;

	//get the first bit
	*dst = *src;
	dstOffset++;

	bool flag;
	if (*src&(0x1 << 7))
	{
		flag = true;
	}
	else
	{
		flag = false;
	}

	u32 i, period;
	bool flag1, flag2;

	period = 1;
	for (i = 1; i<bitsLen; i++)
	{
		flag1 = (src[i / 8] & (1 << (7 - i % 8))) && flag;
		flag2 = (src[i / 8] & (1 << (7 - i % 8))) || flag;

		if (flag1 || !flag2)
		{
			period++;
		}
		else
		{
			elisDeltaCode(period, &dst, &dstOffset);
			period = 1;//reset the length of runs
			flag = !flag;//switch the marker of the runs
		}
	}

	elisDeltaCode(period, &dst, &dstOffset);

	return (dst - savedDst) * 8 + dstOffset;
}

void showElisGammaCode(uchar *buf, int offset)
{
	int total = 0;
	int bitsLen = 0;
	while (!(*buf&(1 << (7 - offset))))
	{
		if (++offset == 8)
		{
			offset = 0;
			buf++;
		}
		bitsLen++;
		total++;

		printf("%c", '0');
		if (total % 8 == 0)
		{
			printf(" ");
		}
	}
	int i;
	for (i = 0; i <= bitsLen; i++)
	{

		if (*buf&(1 << (7 - offset)))
		{
			printf("1");
			total++;
			if (total % 8 == 0)
			{
				printf(" ");
			}
		}
		else
		{
			printf("0");
			total++;
			if (total % 8 == 0)
			{
				printf(" ");
			}
		}
		if (++offset == 8)
		{
			offset = 0;
			buf++;
		}

	}

}
void printBitsForArray(uchar * src, u32 index, u32 len)
{
	u32 words = index >> 3;
	u32 offset = index & 0x7;
	u32 i = 0;
	uchar andx = 128 >> offset;
	int num = 0;
	while (i < len)
	{
		while (andx && i<len)
		{
			if (andx&src[words])
			{
				printf("1");
			}
			else
			{
				printf("0");
			}
			i++;
			if (!(i & 7))
			{
				num++;
				printf(" ");
				if ((num / 8) && (num % 8 == 0))
				{
					cout << " ("<<num / 8<<")" << endl;
				}	
			}
			offset++;
			andx = andx >> 1;
		}
		andx = 128;
		if (!(offset & 7))
		{
			offset = 0;
			words++;
		}
	}
}
void showPlusOneCode(uchar *buf, int offset)
{
	uchar *savedBuf = buf;
	uchar savedOffset = offset;


	u32 bitsLen;
	u32 total = 0;

	//the bitsLen of log(i) + 1
	elisGammaDecode(&bitsLen, &savedBuf, &savedOffset);

	//the bitsLen of log(i)
	bitsLen--;

	showElisGammaCode(buf, offset);
	total = 2 * getBitsNum(bitsLen + 1) + 1;

	u32 i;
	for (i = 0; i<bitsLen; i++)
	{
		if (*savedBuf&(1 << (7 - savedOffset)))
		{
			printf("1");
		}
		else
		{
			printf("0");
		}

		total++;
		if (total % 8 == 0)
		{
			printf(" ");
		}

		if (++savedOffset == 8)
		{
			savedOffset = 0;
			savedBuf++;
		}
	}

}
int integPlusOneCode(u32 *inte,int inteNum,u32 dstSize)
{
	cout << "\n------------plusOne±àÂë(begin£©------------" << endl;
	uchar *plusOneZip = new uchar[dstSize];
	memset(plusOneZip, 0, dstSize);
	uchar *endZip = plusOneZip;
	uchar dstOffset = 0;
	uchar zipOffset = 0;
	for (int i = 0; i < inteNum; i++)
	{
		plusOneCode(inte[i], &endZip, &dstOffset);
	}
	cout << (endZip - plusOneZip) * 8 + dstOffset;
	cout << "\n------------plusOne½âÂë(begin£©------------" << endl;
	uchar *DendZip = plusOneZip;
	uchar DoffsetZip = 0;
	for (int i = 0; i < inteNum; i++)
	{
		u32 num;
		plusOneDecode(&num, &DendZip, &DoffsetZip);
		if (num != inte[i])
		{
			cout << "½âÂë´íÎó" << endl;
		}
	}
	cout << "½âÂë³É¹¦" << endl;
	return 0;
}
int integGammaCode(u32 *inte, int inteNum, u32 dstSize)
{
	cout << "\n------------gamma±àÂë(begin£©------------" << endl;
	uchar *plusOneZip = new uchar[dstSize];
	memset(plusOneZip, 0, dstSize);
	uchar *endZip = plusOneZip;
	uchar dstOffset = 0;
	uchar zipOffset = 0;
	for (int i = 0; i < inteNum; i++)
	{
		elisGammaCode(inte[i], &endZip, &dstOffset);
	}
	cout << (endZip - plusOneZip) * 8 + dstOffset;
	cout << "\n------------gamma½âÂë(begin£©------------" << endl;
	uchar *DendZip = plusOneZip;
	uchar DoffsetZip = 0;
	for (int i = 0; i < inteNum; i++)
	{
		u32 num;
		elisGammaDecode(&num, &DendZip, &DoffsetZip);
		if (num != inte[i])
		{
			cout << "½âÂë´íÎó" << endl;
			return -1;
		}
	}
	cout << "½âÂë³É¹¦" << endl;
	return 0;
}
int integDeltaCode(u32 *inte, int inteNum, u32 dstSize)
{
	cout << "\n------------delta±àÂë(begin£©------------" << endl;
	uchar *plusOneZip = new uchar[dstSize];
	memset(plusOneZip, 0, dstSize);
	uchar *endZip = plusOneZip;
	uchar dstOffset = 0;
	uchar zipOffset = 0;
	for (int i = 0; i < inteNum; i++)
	{
		elisDeltaCode(inte[i], &endZip, &dstOffset);
	}
	cout << (endZip - plusOneZip) * 8 + dstOffset;
	cout << "\n------------delta½âÂë(begin£©------------" << endl;
	uchar *DendZip = plusOneZip;
	uchar DoffsetZip = 0;
	for (int i = 0; i < inteNum; i++)
	{
		u32 num;
		elisDeltaDecode(&num, &DendZip, &DoffsetZip);
		if (num != inte[i])
		{
			cout << "½âÂë´íÎó" << endl;
			return -1;
		}
	}
	cout << "½âÂë³É¹¦" << endl;
	return 0;
}
#if 0
int main()
{
	int arange = 9; 
	for (int i = 1; i < 100; i++)
	{
		arange = i;
		cout << "********************1-" << arange << "********************"<< endl;
		int inteNum = 100000;
		u32 *inte = new u32[inteNum];
		u32 aveLen = (getBitsNum(arange) << 1) + 2;
		int poSize = aveLen*inteNum;
		for (int i = 0; i < inteNum; i++)
		{
			inte[i] = rand() % arange + 1;
		}
		integPlusOneCode(inte, inteNum, poSize);
		integGammaCode(inte, inteNum, poSize);
		integDeltaCode(inte, inteNum, poSize);
		cout << "I am happy!" << endl;
	}
	
	return 0;
}
#endif