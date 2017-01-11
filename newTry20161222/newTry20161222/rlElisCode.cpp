#include "rlElisCode.h"
#include "wzip.h"
# include <memory.h>
#include <math.h>

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
int runLengthGammaCode(uchar *src, u32 bitsLen, uchar *dst)
{
	int ret;
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
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
int elisGammaCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	int ret;
	if (!num || !buffPPtr || !offset)
	{
		return ERR_PARAMETER;
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

int runLengthGammaDecode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}
	int i_bai = 0;
	uchar srcOffset;
	uchar dstOffset;

	uchar *savedSrc = src;
	uchar *savedDst = dst;


	bool flag = src[0] & (1 << 7) ? true : false;
	*dst = *src;

	srcOffset = 1;
	dstOffset = 0;

	u32 num;
	u32 i;
	while ((src - savedSrc) * 8 + srcOffset
		<
		bitsLen
 		)
	{
		i_bai++;
		if (i_bai > 9578)
		{
			int baiai = 0;
		}
		//cout << "baibaizai" << i_bai << endl;
		if (i_bai % 10==0)
		{
			int xxxxx = 10;
			//cout << "baibaizai" << i_bai << endl;
			if (i_bai % 100 == 0)
			{
				//cout << "\t\tbaibaizai" << i_bai << endl;
				int xxxxx = 100;
				if (i_bai % 1000 == 0)
				{
					int xxxxx = 1000;
					//cout << "\tbaibaizai" << i_bai << endl;
					if (i_bai % 10000 == 0)
					{
						int xxxxx = 1000;
						//cout <<"baibaizai"<< i_bai << endl;
					}
				}
			}
		}
		elisGammaDecode(&num, &src, &srcOffset);
		if (flag)
		{
			// 1 runs
			for (i = 0; i<num; i++)
			{
				*dst |= (1 << (7 - dstOffset));
				if (++dstOffset == 8)
				{
					dstOffset = 0;
					dst++;
				}
			}
		}
		else
		{
			//0 runs
			for (i = 0; i<num; i++)
			{
				*dst &= ~(1 << (7 - dstOffset));
				if (++dstOffset == 8)
				{
					dstOffset = 0;
					dst++;
				}
			}
		}

		flag = !flag;
	}
	return (dst - savedDst) * 8 + dstOffset;
}


int elisDeltaCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	if (!num || !buffPPtr || !offset)
	{
		return ERR_PARAMETER;
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

void showElisDeltaCode(uchar *buf, int offset)
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

int runLengthDeltaCode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
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

int runLengthDeltaDecode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}

	uchar srcOffset;
	uchar dstOffset;

	uchar *savedDst = dst;
	uchar *savedSrc = src;

	bool flag;
	//get the first bit
	*dst = *src;
	if (*src&(1 << 7))
	{
		flag = true;
	}
	else
	{
		flag = false;
	}

	//
	u32 i, count;

	srcOffset = 1;
	dstOffset = 0;

	while ((src - savedSrc) * 8 + srcOffset
		<bitsLen
		)
	{
		elisDeltaDecode(&count, &src, &srcOffset);
		if (flag)
		{
			// the runs of 1s 
			for (i = 0; i<count; i++)
			{
				*dst |= (1 << (7 - dstOffset));
				if (++dstOffset == 8)
				{
					dstOffset = 0;
					dst++;
				}
			}
		}
		else
		{
			//the runs of 0s
			for (i = 0; i<count; i++)
			{
				*dst &= ~(1 << (7 - dstOffset));
				if (++dstOffset == 8)
				{
					dstOffset = 0;
					dst++;
				}
			}
		}

		flag = !flag;//switch the marker of runs
	}
	return (dst - savedDst) * 8 + dstOffset;
}

//-----------------------baiwenwu--------------------

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
	*num = sum - 1;
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
int runLengthPlusOneDecode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}

	uchar srcOffset;
	uchar dstOffset;

	uchar *savedDst = dst;
	uchar *savedSrc = src;

	bool flag;
	//get the first bit
	*dst = *src;
	if (*src&(1 << 7))
	{
		flag = true;
	}
	else
	{
		flag = false;
	}

	//
	u32 i, count;

	srcOffset = 1;
	dstOffset = 0;

	while ((src - savedSrc) * 8 + srcOffset
		<bitsLen
		)
	{
		plusOneDecode(&count, &src, &srcOffset);
		if (flag)
		{
			// the runs of 1s 
			for (i = 0; i<count; i++)
			{
				*dst |= (1 << (7 - dstOffset));
				if (++dstOffset == 8)
				{
					dstOffset = 0;
					dst++;
				}
			}
		}
		else
		{
			//the runs of 0s
			for (i = 0; i<count; i++)
			{
				*dst &= ~(1 << (7 - dstOffset));
				if (++dstOffset == 8)
				{
					dstOffset = 0;
					dst++;
				}
			}
		}

		flag = !flag;//switch the marker of runs
	}
	return (dst - savedDst) * 8 + dstOffset;
}

u32 EndWords;//for getruns function
uchar bits[256] = {//run length gamma
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
uchar getFirstBit(uchar *src, u32 index)
{//runlength gamma
	u32 words = index >> 3;
	u32 offset = index & 0x7;
	return (src[words] & (128 >> offset));
}
u32 getRuns(uchar *src, u32 index)
{//runlength gamma

	u32 words = index >> 3;
	u32 offset = (index & 7);
	uchar ch_t = src[words] << offset;
	u32 num = bits[ch_t];
	if (num < 8 - offset){
		return num;
	}
	else{
		num = 8 - offset;
		while (words<EndWords){
			ch_t = src[words++] & 1;
			uchar bits_r = bits[src[words]];

			if (bits_r< 8 && (ch_t == (src[words] >> 7))){
				return num + bits_r;
			}
			else if (bits_r == 8 && (ch_t == (src[words] >> 7))){
				num += 8;
			}
			else{
				return num;
			}
		}
	}
	return 0;
}
void Append_g(uchar*cdata, u32 index, u32 runs)
{//runlength gamma
	u32 zerosNum = getBitsNum(runs);
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
void copyBits_c(uchar * src, u32 src_index, uchar *cdata, u32 cd_index, u32 len)
{//plain
	u32 numBits = 0;
	u32 src_words = src_index >> 3;
	uchar src_offset = src_index & 0x7;
	u32 cd_words = cd_index >> 3;
	uchar cd_offset = cd_index & 0x7;
	uchar offset;
	numBits += (8 - src_offset);
	if (src_offset >= cd_offset)//原串剩余< 压缩串剩余(src剩余<cdata剩余)
	{
		uchar ch_t = src[src_words] << src_offset;
		cdata[cd_words] += (ch_t >> cd_offset);
		offset = 8 - (src_offset - cd_offset);
	}
	else
	{
		uchar ch_t = src[src_words] << src_offset;
		cdata[cd_words++] += (ch_t >> cd_offset);
		offset = cd_offset - src_offset;
		cdata[cd_words] += (ch_t << (8 - cd_offset));

	}
	if (offset == 8)
	{
		for (u32 i = 0; i < (len >> 3); i++)
			cdata[++cd_words] = src[++src_words];
		cdata[cd_words] = cdata[cd_words] >> (8 - cd_offset);
		cdata[cd_words] = cdata[cd_words] << (8 - cd_offset);
		return;
	}
	while (numBits < len)
	{
		uchar ch_t = src[++src_words];
		cdata[cd_words++] += (ch_t >> offset);
		cdata[cd_words] = (ch_t << (8 - offset));
		numBits += 8;
	}
	//结尾处理(cdata数组中，多加入了[8-src_offset]位数据，需要清除)
	cd_words = (cd_index + len) >> 3;
	uchar cd_offset11 = (cd_index + len) & 0x7;
	uchar cd_ch_t = cdata[cd_words] >> (8 - cd_offset);
	cdata[cd_words++] = cd_ch_t << (8 - cd_offset);
	cdata[cd_words] = 0;
}

int runLengthHybirdCode(waveletTree wavTree, u32 HBblockSize)
{
	EndWords = (wavTree->bitLen >> 3) + ((wavTree->bitLen & 0x7) ? 1 : 0);
	u32 HB_blockBits = getBitsNum(HBblockSize);
	u32 HB_blockMask = (1u << HB_blockBits) - 1;
	wavTree->head = new bitArray(wavTree->bitLen, 3, HBblockSize);
	//wavTree->headNum = (wavTree->bitLen / HBblockSize)>1;
	//wavTree->head = createHeadArray(wavTree->headNum);
	u32 headIndex = 0;
	u32 index = 0;
	uchar firstBit;
	u32 runs_t = 0;
	u32 *runsArray = (u32*)malloc(sizeof(u32)*HBblockSize);
	//for test 
	u32 headNums = 0;
	if (wavTree->bitLen == 47152)
	{
		int xxx = 0;
	}
	while (index<wavTree->bitLen)
	{
		headNums++;
		u32 cpIndex = index;
		u32 rl_gamma = 0;
		if (index == wavTree->bitLen)
			break;
		u32 bits = 0;
		firstBit = getFirstBit(wavTree->bitBuff, index);
		memset(runsArray, 0, HBblockSize*sizeof(u32));
		int k = 0;
		while (bits<HBblockSize&&index < wavTree->bitLen)
		{
			runs_t = getRuns(wavTree->bitBuff, index);
			bits += runs_t;
			index += runs_t;
			runsArray[k] = runs_t;
			k++;
		}
		if (k == 1)
		{
			u32 bitAll01 = (bits >> HB_blockBits) * 3 + (getBitsNum(bits&HB_blockMask) << 1) + 1;
			if (bitAll01 < 23)
			{
				if (bitAll01 > 23)
				{
					u32 xxxx = bitAll01 - 23;
					AllXRuns += xxxx;
					if (xxxx == 131)
					{
						cout << AllXRuns << endl;
					}
				}
				index = index - ((bits > HBblockSize) ? (bits - HBblockSize) : 0);
				if (firstBit){//ALL1存储
					wavTree->head->writeValue(1);
				}
				else{//ALL0存储
					wavTree->head->writeValue(0);
				}
				
			}
			else{
				if (firstBit){//ALL1存储
					wavTree->head->writeValue(5);
				}
				else{//ALL0存储
					wavTree->head->writeValue(6);
				}
				wavTree->zipLen +=23;
#if 1
				u32 xxxx = bitAll01 - 23;
				AllXRuns += xxxx;
				cout << AllXRuns << endl;
#endif 
			}
			continue;
		}
		else
		{
			
			for (int i = 0; i<k-1; i++)
				rl_gamma += getBitsNum(runsArray[i]);
			rl_gamma = (rl_gamma << 1) + k;
			u32 rl_gamma_t = rl_gamma;
			rl_gamma += getBitsNum(runsArray[k - 1])<<1;
			if (rl_gamma > bits || rl_gamma_t+6>HBblockSize)
			{//Plain
				/*
				for (int i = 0; i < k; i++)
				{
					cout << runsArray[i] << " ";

				}
				cout << endl;
				if (bits < HBblockSize)
				{
					int xxx = 0;
				}*/
				index = index - ((bits > HBblockSize) ? (bits - HBblockSize) : 0);
				wavTree->head->writeValue(2);
				u32 WriteLen = index - cpIndex;
				copyBits_c(wavTree->bitBuff, cpIndex, wavTree->zipBuff, wavTree->zipLen, WriteLen);
				wavTree->zipLen += WriteLen;
				continue;
			}
			u32 bits_1 = bits - HBblockSize;
			if (bits_1 > HBblockSize)
			{
				u32 Bits_t = (getBitsNum(runsArray[k - 1] - bits_1)<<1) +
					(bits_1 >> HB_blockBits) * 3 + (getBitsNum(bits_1&HB_blockMask)<<1)+1;
				if (Bits_t < rl_gamma - rl_gamma_t)
				{
					index = index - (bits - HBblockSize);
					runsArray[k - 1] -= bits_1;
#if 1
					rl_gamma_t += (getBitsNum(runsArray[k - 1]) << 1);
					if (rl_gamma_t > HBblockSize)
					{
						cout << "算法逻辑不是很严密，考虑得不周全，应该将细节规整一下！" << endl;
					}
#endif
				}
			}
			if (firstBit)
			{
				wavTree->head->writeValue(3);
			}
			else
			{
				wavTree->head->writeValue(4);
			}
			for (int i = 0; i < k; i++)
			{
				Append_g(wavTree->zipBuff, wavTree->zipLen, runsArray[i]);
				wavTree->zipLen += (getBitsNum(runsArray[i]) << 1) + 1;
			}
		}
	}
	if (headNums == 44)
	{
		int xxx = 0;
	}
	return 0;
}
//----------------hybirdDecode---------------------
int hybirdDecode(waveletTree root, u32 HBblockSize)
{
	u32 deZipIndex = 0;

	return 0;
}
/*int hybirdDecode(waveletTree root, u32 HBblockSize)
{

	uchar *tmp_src;
	u32 tmp_srcLen = 0;
	u32 decLen = 0;
	u32 errs;
	creatUcharArr(&tmp_src, node->srcNum + 36);
	u32 i = 0;
	int block_i = 0;
	while (tmp_srcLen < node->srcLen)
	{
		switch (node->head->getValue_i(i++))
		{
		case 0:
			break;
		case 1:
			writeAll1(tmp_src, tmp_srcLen);
			break;
		case 2:
			copyBits(node->cdata, decLen, tmp_src, tmp_srcLen, blockSize);
			decLen += blockSize; break;
		case 3:
			decLen = decodeGamma(node->cdata, decLen, tmp_src, tmp_srcLen, 1);
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
}*/