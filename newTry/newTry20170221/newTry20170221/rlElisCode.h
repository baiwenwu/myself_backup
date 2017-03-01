#ifndef _RLE_ELIS_CODE_H
#define _RLE_ELIS_CODE_H
#include "wzip.h"
#include "wavelet.h"

int getBitsNum(u32 number);


int elisGammaCode(u32 num, uchar **buffPPtr, uchar *offset);

int elisGammaDecode(u32 *num, uchar **buffPPtr, uchar* offset);

void showElisGammaCode(uchar *buf, int offset);

int runLengthGammaCode(uchar *src, u32 bitsLen, uchar *dst);

int runLengthGammaDecode(uchar *src, u32 bitsLen, uchar *dst);


int elisDeltaCode(u32 num, uchar **buffPPtr, uchar *offset);

int elisDeltaDecode(u32 *num, uchar **buffPPtr, uchar* offset);

void showElisDeltaCode(uchar *buf, int offset);

int runLengthDeltaCode(uchar *src, u32 bitsLen, uchar *dst);

int runLengthDeltaDecode(uchar *src, u32 bitsLen, uchar *dst);


int plusOneCode(u32 num, uchar **buffPPtr, uchar *offset);

int plusOneDecode(u32 *num, uchar **buffPPtr, uchar* offset);

int runLengthPlusOneCode(uchar *src, u32 bitsLen, uchar *dst);

int runLengthPlusOneDecode(uchar *src, u32 bitsLen, uchar *dst);


//int runLengthHybirdCode(waveletTree wavTree, u32 HBblockSize);
//int hybirdDecode(waveletTree root, u32 HBblockSize);

#endif