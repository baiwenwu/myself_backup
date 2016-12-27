# pragma once
#include "baseLib.h"
#include "bitArray.h"
struct fileStream{
	uchar *src;
	u32 srcLen;
	u32 srcNum;
	struct bitArray *head;
	uchar *cdata; 
	u32 cdLen;
	u32 cdNum;
	fileStream();
};
static u32  blockSize=512;

void printBitsOfByte(uchar ch);
int printString(uchar *str, u32 sIndex,int num);
void printBitsForArray
	(uchar * src, u32 index, u32 len);
void randFillUcharArr
	(uchar *arr, u32 len);
void creatUcharArr
	(uchar **arr, u32 len);
int testCopyBitsFun(uchar *src, u32 srcLen);
uchar getOneUcharFromArr(uchar *src, u32 index);
void copyBits(uchar * src, u32 src_index, uchar *cdata,
	                 u32 cd_index, u32 len);


//-----------文件操作相关函数-----------
int writeToFile(fileStream *s);
int readFromFile(fileStream *s);
bool isEqual(fileStream *x, fileStream *y);
int writeSrcFile(uchar *str, u32 len);