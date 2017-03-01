#include"f_test.h"
int fileOpenTest(char *fileName)
{
	ifstream in;
	in.open("management-server.log");
	if (!in.is_open())
	{
		cout << "open file error!" << endl;
		return -1;
	}
	char buff[100];
	in.read(buff, 100);
	for (int i = 0; i < 100; i++)
		cout << buff[i];
	cout << endl;
	cout << "------test success------" << endl;
	return 0;
}
int testTellg(char * fileName)
{
	ofstream out;
	out.open(fileName, ios::out);
	char *str = "i think i love you very much,and i hope you have the same motion as me!/";
	out.write(str, strlen(str));
	cout<<"tellp="<<out.tellp()<<endl;
	out.close();
	ifstream in;
	in.open(fileName, ios::in);

	while (1)
	{
		char ch_t[3];
		in.read(ch_t, 2);
		ch_t[2] = '\0';
		cout << ch_t << "  tellg=" << in.tellg() << "  count=";
		cout << in.gcount() << endl;

	}
	in.close();
}
#if 0
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
				wavTree->zipLen += 23;
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

			for (int i = 0; i<k - 1; i++)
				rl_gamma += getBitsNum(runsArray[i]);
			rl_gamma = (rl_gamma << 1) + k;
			u32 rl_gamma_t = rl_gamma;
			rl_gamma += getBitsNum(runsArray[k - 1]) << 1;
			if (rl_gamma > bits || rl_gamma_t + 6>HBblockSize)
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
				u32 Bits_t = (getBitsNum(runsArray[k - 1] - bits_1) << 1) +
					(bits_1 >> HB_blockBits) * 3 + (getBitsNum(bits_1&HB_blockMask) << 1) + 1;
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
#endif 
