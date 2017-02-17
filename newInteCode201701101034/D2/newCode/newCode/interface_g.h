#ifndef _INTERFACE_G_H_
#define _INTERFACE_G_H_
#include"type.h"
uchar getOneUchar(uchar *src, u32 index);
uchar getNextOneBit(uchar *src, uchar offset);
uchar getMark2(uchar **src, uchar *offset);
void writeMark2(uchar **src, uchar *offset, uchar val);
u32 getRuns(uchar **src, uchar *offset);
void writeRuns(uchar **src, uchar *offset, u32 runs);
int getBitsOfNum(unsigned x);//��log(x)����ȡ��
void Append_g(u32 runs, uchar **buffPPtr, uchar *offset);
void bitsCopy(uchar **dst, uchar *dstOff, uchar **src,
	uchar *srcOff, u32 len);//bits copy
int getSubGP(u32 x);
int creatSrc(uchar *src, u32& bitsLen);
extern uchar *EndWords;
extern uchar EndOff;
extern u16 plusOlen[512];
#endif