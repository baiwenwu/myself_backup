#ifndef  _DECOMPRESS_H
#define  _DECOMPRESS_H
#include "wzip.h"

#include"wavelet.h"
#include"fileProcess.h"
int streamBlkDecompressInit(Stream_t *streamPtr);
int paraseBlkCharSetMap(Stream_t *streamPtr);
int paraseBlkCharCodeTable(Stream_t *streamPtr);
int paraseBlkBwtIndex(Stream_t *streamPtr);
//waveletTree genWavtreeWithCodeTable(char(*codeTable)[CODE_MAX_LEN]);
waveletTree genWavtreeWithCodeTable(char codeTable[CHAR_SET_SIZE][CODE_MAX_LEN]);
int paraseBlkZipNodeWithPreorder(waveletTree root,Stream_t *streamPtr);
int genBwtWithWaveletTree(waveletTree root, Stream_t *streamPtr);
int genOrigBlkWithBwt(uchar *bwt, u32 len, u32 bwtIndex, uchar *orig);
int streamWriteOrigBlk(Stream_t *streamPtr);
int streamBlkDecompressCleanUp(Stream_t *streamPtr);
extern int keepOrigFile;//保存源文件
extern int overWrite;
#endif