#pragma once
#include "global.h"
//#include "baseLib.h"
//#include "bitArray.h"
//int runLengthHybirdCode(uchar *src, u32 bitsLen, uchar **head, u32 *headLen, uchar *dst);
int getBitsNum(unsigned x);
uchar getFirstBit(uchar src, u32 index);
u32 getRuns(uchar *src, u32 index,uchar flag);
//for tes
void showGamma(uchar *src, u32 srcLen);
//for test
void Append_g(uchar*cdata, u32 index, u32 runs);
int runLengthHybirdCode(fileStream *node);