#include<fstream>
#include "global.h"

fileStream::fileStream()
{
	src = NULL;
	srcLen = 0;
	srcNum = 0;
	head = NULL;
	cdata = NULL;
	cdLen = 0;
	cdNum = 0;
}
//for test 
void printBitsOfByte(uchar ch)
{
	uchar andx = 1 << 7;
	while (andx)
	{
		if (andx&ch)
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
		andx = (andx >> 1);
	}
}
// for test 
void printBits(u64 val, uchar len)
{
	u64 andx = 1ULL << (len - 1);
	for (uchar i = 0; i < len; i++)
	{
		if (val&andx)
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
		
	}
}
// for test
void printBitsForArray(uchar * src, u32 index, u32 len)
{
	u32 words = index >> 3;
	u32 offset = index & 0x7;
	u32 i = 0;
	uchar andx = 128 >> offset;
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
				printf(" ");
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
// for test
int printString(uchar *str, u32 sIndex, int num)
{
	if (!str || !num)
	{
		return -1;
	}
	int x = sIndex >> 3;
	int i = x;
	u32 endlNum = 0;
	//while (str[i] != '\0' && i < num)
	while (i < num+x)
	{
		endlNum++;
		printBitsOfByte(str[i++]);
		if (!(endlNum & 0x7))
			cout << endl;
		else
			cout << " ";
	}
}
void randFillUcharArr(uchar *arr, u32 len)
{
	if (!arr)
	{
		printf("生成随机字符串失败!\n");
		return;
	}
	//srand(time(0));
	u32 i = 0;
	for (; i < len; i++)
	{
		//arr[i] = rand() % 25 + 'a';
		switch (rand() % 7)
		{
		case 0:arr[i] = rand() % 255; break;
		case 1: arr[i] = 'A'; break;
		case 2: arr[i] = 'C'; break;
		case 3: arr[i] = 'G'; break;
		case 4: arr[i] = 'T'; break;
		//case 5: arr[i] = 'A'; break;
		default: arr[i] = 0;
			break;
		}
	}
	arr[len] = '\0';
}
void creatUcharArr(uchar **arr, u32 len)
{
	*arr = (uchar *)malloc(len*sizeof(uchar));
	memset(*arr, 0, len*sizeof(uchar));
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
uchar getOneUcharFromArr(uchar *src, u32 index)
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
int testCopyBitsFun(uchar *src, u32 srcLen)
{
	uchar *cdata;
	creatUcharArr(&cdata, (srcLen >> 3) + 30);
	while (1)
	{
		u32 src_index = rand() % 328;
		u32 cd_index = rand() % 299;
		copyBits(src, src_index, cdata, cd_index, 256);
		for (u32 i = 0; i < 32; i++)
		{
			if (getOneUcharFromArr(src, src_index) != getOneUcharFromArr(cdata, cd_index))
			{
				printf("%d \twrite plain code error!\n", i);
				//exit(1);
				break;
			}
			src_index += 8;
			cd_index += 8;
		}
		memset(cdata, 0, sizeof(uchar)*((srcLen >> 3) + 30));
	}

}



int writeToFile(fileStream *s)
{
	if (!s)
	{
		cout << "the parameter is error!" << endl;
		return -1;
	}
	//定义文件输出流   文件不存在时创建文件  
	//ofstream outFile("xb1.dat", ios::out | ios::binary);  
	ofstream outFile("xb1.txt", ios::out);
	//对文件打开错误时的操作  
	if (!outFile)
	{
		cout << "The file open error!" << endl;
		return -2;
	}
	cout << "fileStream=" << sizeof(fileStream) << endl;
	outFile.write((char*)s, sizeof(fileStream));
	cout << "struct_head=" << sizeof(bitArray) << endl;
	outFile.write((char*)s->head, sizeof(bitArray));
	cout << "head=" << sizeof(uchar)*s->head->arrLen << endl;
	outFile.write((char*)s->head->arr, sizeof(uchar)*s->head->arrLen);
	cout << "cdata=" << sizeof(uchar)*s->cdNum << endl;
	outFile.write((char*)s->cdata, sizeof(uchar)*s->cdNum);
	//char *test = "wo ai baiwenwu!";
	//outFile.write((char*)test, sizeof(uchar)*strlen(test));
	//outFile.write((char*)test, sizeof(uchar)*strlen(test));
	outFile.close();
	return 0;
}
int readFromFile(fileStream *s)
{
	if (!s)
	{
		cout << "The file open error!" << endl;
		return -2;
	}
	//文件输入流  将文件中的student信息读出到屏幕上  
	//ifstream inFile("xb1.dat", ios::in | ios::binary);
	ifstream inFile("xb1.txt", ios::out);
	if (!inFile)
	{
		cout << "The inFile open error!" << endl;
		return -1;
	}
	inFile.read((char*)s, sizeof(fileStream));
	s->head = new bitArray(0,2);
	inFile.read((char*)s->head, sizeof(bitArray));
	s->head->arr = new uchar[s->head->arrLen];
	inFile.read((char*)s->head->arr, sizeof(uchar)*s->head->arrLen);
	s->cdata = new uchar[s->cdNum];
	inFile.read((char*)s->cdata, sizeof(uchar)*s->cdNum);

	inFile.close();
	return 0;
}
bool isEqual(fileStream *x, fileStream *y)
{
	if ( x->cdLen != y->cdLen || x->cdNum != y->cdNum
	 || x->src != y->src || x->srcLen != y->srcLen || x->srcNum != y->srcNum)
	{
		return false;
	}
	bitArray *headx = x->head;
	bitArray *heady = y->head;
	return true;
}
int writeSrcFile(uchar *str, u32 len)
{
	if (!str)
	{
		cout << "The file open error!" << endl;
		return -2;
	}
	ofstream outFile("srcStr.txt", ios::out);  //定义文件输出流   文件不存在时创建文件  
	//对文件打开错误时的操作  
	if (!outFile)
	{
		cout << "The file open error!" << endl;
		return -2;
	}
	outFile.write((char*)str, sizeof(uchar)*len);
	outFile.close();
	return 0;
}