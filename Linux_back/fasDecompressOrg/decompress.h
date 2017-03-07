#ifndef  _DECOMPRESS_H
#define  _DECOMPRESS_H
#include "wzip.h"

int streamBlkDecompressInit(Stream_t *streamPtr);

int streamBlkDecompressCleanUp(Stream_t *streamPtr);

int streamBlkDecompressNew(Stream_t *streamPtr);

int paraseFileHeader(Stream_t *streamPtr);

int paraseFileTail(FILE *zipFile,Stream_t *streamPtr);

int paraseBlkSiz(Stream_t *streamPtr);

int paraseNodeCodeType(Stream_t *streamPtr);

int paraseHBblockSize(Stream_t *streamPtr,u32 &HBblockSize);

int paraseBlkCharSetMap(Stream_t *streamPtr);

int paraseBlkCharCodeTable(Stream_t *streamPtr);

int paraseBlkBwtIndex(Stream_t *streamPtr);

waveletTree genWavtreeWithCodeTable(char (*codeTable)[CODE_MAX_LEN]);

int paraseBlkZipNodeWithPreorder(Stream_t *streamPtr,u32 HBblockSize);
int genBwtWithWaveletTree(waveletTree root,Stream_t *streamPtr);

int genOrigBlkWithBwt(uchar *bwt,u32 len,u32 bwtIndex,uchar *orig);

int streamWriteOrigBlk(Stream_t *streamPtr);

void decompressMain(Stream_t *streamPtr);
#endif
