#include"compress.h"
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

uchar bits[256] = {
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
{
	u32 words = index >> 3;
	u32 offset = (index & 7);
	return src[words] & (128>offset);
}
u32 getRuns(uchar *src, u32 index)
{// 1 bug occur when offset equals 0
	
		u32 words = index >> 3;
		u32 offset = (index & 7);
		uchar ch_t = src[words] << offset;
		u32 num = bits[ch_t];
		if ( num < 8 - offset)
		{
			return num;
		}
		else
		{
			num = 8 - offset;
			while (1)
			{
				ch_t = src[words++] & 1;
				uchar bits_r = bits[src[words]];

				if (bits_r< 8 && (ch_t == (src[words]>>7)))
				{
					return num + bits_r;
				}
				else if (bits_r == 8 && (ch_t == (src[words] >> 7)))
				{
					num += 8;
				}
				else
				{
					return num;
				}
			}
		}
		return 0;
}
//for test
void showGamma(uchar *src, u32 srcLen)
{
	u32 index = 0;
	
	while (index <srcLen)
	{
		u32 runs = getRuns(src, index);
		printBitsForArray(src, index, runs);
		cout << "\n runs=" << runs << endl;
		index += runs;
	}
}
void testApend_g()
{
	uchar *cdata;
	u32 index = 0;
	creatUcharArr(&cdata, 10000);
	for (int i = 0; i < 50; i++)
	{
		u32 runs = rand() % 2048;
		Append_g(cdata, index, runs);
		u32 len = (getBitsNum(runs) << 1) + 1;
		cout << runs << endl;
		printBitsForArray(cdata, index, len);
		cout << endl;
		index += len;
	}
}
/*
此处考虑传指针比较好，把最后一个words记录下来，同时让Index在调用函数内部完成自动加的操作
*/
void Append_g1(uchar*cdata, u32 *index, u32 runs, u32 *wordsL)
{//个人觉得这个方法很好
	*index += (getBitsNum(runs) << 1) + 1;
	u32 wordsR = *index >> 3;
	u32 offset = *index & 0x111;
	if (!offset)
		wordsR--;
	runs = runs << (8 - offset);
	while (wordsR > *wordsL)
	{
		cdata[wordsR] += runs & 0xff;
		runs = runs >> 8;
		wordsR--;
	}
	cdata[wordsR] += runs & 0xff;
	runs = runs >> 8;
}
void Append_g(uchar*cdata, u32 index, u32 runs)
{//个人觉得这个方法很好
	u32 zerosNum = getBitsNum(runs);
	index += zerosNum;
	u32 wordsL = index >> 3;
	index += zerosNum + 1;
	u32 wordsR = index >> 3;
	u32 offset = index & 0x111;
	if (!offset)
		wordsR--;
	runs = runs << (8 - offset);
	while (wordsR > wordsL)
	{
		cdata[wordsR] += runs & 0xff;
		runs = runs >> 8;
		wordsR--;
	}
	cdata[wordsR] += runs & 0xff;
	runs = runs >> 8;
}
void copyBits(uchar * src, u32 src_index, uchar *cdata, u32 cd_index, u32 len)
{
	u32 numBits = 0;
	u32 src_words = src_index >> 3;
	uchar src_offset = src_index & 0x7;
	u32 cd_words = cd_index >> 3;
	uchar cd_offset = cd_index & 0x7;
	uchar offset;
	numBits += (8 - src_offset);
	if (src_offset >= cd_offset)//src剩余< cd 剩余
	{
		uchar ch_t = src[src_words] << src_offset;
		cdata[cd_words] += (ch_t >> cd_offset);
		offset = 8-(src_offset - cd_offset);
	}
	else
	{
		uchar ch_t = src[src_words] << src_offset;
		cdata[cd_words++] += (ch_t >> cd_offset);
		offset = cd_offset - src_offset;
		cdata[cd_words] += (ch_t << (8-cd_offset));
		
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

	/*printBitsForArray(src, src_index+256, 16);
	cout << "\t" << int(8-src_offset);
	cout << endl;
	printBitsForArray(cdata, cd_index+256, 16);
	cout << "\t" << int(cd_offset);
	cout << endl;
	uchar tmp_ch = getOneUcharFromArr(cdata, cd_index + len);
	if (tmp_ch)
	{
		printBitsOfByte(tmp_ch);
	}
	cout << endl;*/
}
uchar getOneUcharFromArr(uchar *src,u32 index)
{
	u32 words = index >> 3;
	u32 offset = index & 0x7;
	if (!offset)
	{
		return src[words];
	}
	uchar ch_t = src[words++] << offset;
	ch_t += src[words] >> (8 - offset);
	return ch_t;
}
int testCopyBitsFun(uchar *src,u32 srcLen)
{
	uchar *cdata;
	creatUcharArr(&cdata, (srcLen>>3) + 30);
	while (1)
	{
		u32 src_index = rand() % 328;
		u32 cd_index = rand() % 299;
		copyBits(src, src_index, cdata, cd_index, 256);
		for (u32 i = 0; i < 32; i++)
		{
			if (getOneUcharFromArr(src, src_index) != getOneUcharFromArr(cdata, cd_index))
			{
				printf("%d \t error!\n",i);
				//exit(1);
				break;
			}
			src_index += 8;
			cd_index += 8;
		}
		memset(cdata, 0, sizeof(uchar)*((srcLen >> 3) + 30));		
	}
	
}
int runLengthHybirdCode(fileStream *node)
{

	if (!node->src||!node->cdNum||!node->cdata||!node->cdLen||!node->head)
	{
		cout << "params of runLengthHybirdCode is error!" << endl;
		return -1;
	}
	//cereate head array
	node->head = new bitArray(node->srcLen,3);
	//create cdata array
	creatUcharArr(&node->cdata, node->srcNum);

	u32 index = 0;
	uchar firstBit;
	u32 runs_t = 0;
	u32 *runsArray = (u32*)malloc(sizeof(u32)*blockSize);
	while (index<node->srcLen)
	{
		u32 rl_gamma = 0;
		if (index == node->srcLen)
			break;
		u32 bits = 0;
		firstBit = getFirstBit(node->src, index);
		memset(runsArray, 0, blockSize*sizeof(u32));
		int k = 0;
		while (bits<blockSize&&index < node->srcLen)
		{
			runs_t = getRuns(node->src, index);
			bits = bits + runs_t;
			runsArray[k] = runs_t;
			k++;
		}
		if (bits>blockSize)
		{//调整位一个确定长度
			index = index - (bits - blockSize);
			runsArray[k - 1] -= (bits - blockSize);
		}
		int i = 0;
		//计算出gamma编码需要的长度
		for (; i<k; i++)
			rl_gamma += getBitsNum(runsArray[i]);
		rl_gamma = (rl_gamma << 1) + k;

		//可以建立一定的松弛度,也可以不建立
		//u32 thred=20;
		if (k == 1)
		{
			if (firstBit)
			{//ALL1存储
				node->head->writeValue(1);
			}
			else
			{//ALL0存储
				node->head->writeValue(0);
			}
			index += blockSize;
		}
		else if (rl_gamma>=blockSize || index == node->srcLen)
		{//plain
			node->head->writeValue(2);

		}
		else
		{//rl_gmma
			if (firstBit)
			{
				node->head->writeValue(3);
			}
			else
			{
				node->head->writeValue(4);
			}
			for (int i = 0; i < k; i++)
			{
				//Append_g(node->cdata, index, runsArray[k]);
				//index += runsArray[k];
			}
		}
	}
	return 0;
}