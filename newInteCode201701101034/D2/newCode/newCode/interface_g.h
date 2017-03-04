#ifndef _INTERFACE_G_H_
#define _INTERFACE_G_H_
#include"type.h"
uchar getOneUchar(uchar *src, u32 index);
uchar getNextOneBit(uchar *src, uchar offset);
uchar getMark2(uchar **src, uchar *offset);
void writeMark2(uchar **src, uchar *offset, uchar val);
u32 getRuns(uchar **src, uchar *offset);
void writeRuns(uchar **src, uchar *offset, u32 runs);
void writeRuns_t(uchar *src, uchar offset, u32 runs);
int getBitsOfNum(unsigned x);//求log(x)向下取整
void Append_g(u32 runs, uchar **buffPPtr, uchar *offset);
void bitsCopy(uchar **dst, uchar *dstOff, uchar **src,
	uchar *srcOff, u32 len);//bits copy
int getSubGP(u32 x);
int creatSrc(uchar *src, u32& bitsLen);
extern uchar *EndWords;
extern uchar EndOff;
extern u16 plusOlen[512];
extern u16 degaTab[512];
u16 getBitsPO(uchar *src, uchar srcOff);
u32 getBitsPO1(uchar *src, uchar srcOff, u16 len);
uchar getRunsForDeg(uchar **src, uchar *srcOff);
int runLengthGammaDecode_228(uchar *src, u32 bitsLen, uchar *dst);

void deCodeGamma_224(bool *flag, uchar **src, uchar *srcOff,
	uchar **dst, uchar *dstOff, u32 HBblSize);
#endif