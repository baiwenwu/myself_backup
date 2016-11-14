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
int printString(uchar *str, int num)
{
	if (!str || !num)
	{
		return -1;
	}
	int i = 0;
	//while (str[i] != '\0' && i < num)
	while (i < num)
	{
		printBitsOfByte(str[i++]);
	}
}
void randFillUcharArr(uchar *arr, u32 len)
{
	if (!arr)
	{
		printf("Éú³ÉËæ»ú×Ö·û´®Ê§°Ü!\n");
		return;
	}
	//srand(time(0));
	u32 i = 0;
	for (; i < len; i++)
	{
		//arr[i] = rand() % 25 + 'a';
		switch (rand() % 5)
		{
		case 0:arr[i] = rand() % 255; break;
		case 1: arr[i] = 0; break;
		case 2: arr[i] = 1; break;
		case 3: arr[i] = 1; break;
		case 4: arr[i] = 0; break;
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