#ifndef _COMPRESS_H
#define _COMPRESS_H
#include "wavelet.h"
#include "wzip.h"

int getBlockDataInfo(uchar *inbuff, u32 len,	bool *charMap, u32 *charFreq,	u32 *setSiz	);
int getBlockSizeForHybirdCode(uchar *inbuff, u32 len, u32 &BlockSize, uchar level);



int writeFileHeader(ofstream &oufd);
//int treeCode(Stream_t *streamPtr);
int writeFileEnd(ofstream &zipfd);


//int writeCompressArg(FILE *zipFile, Stream_t *streamPtr);

int writeBlkCharSetMap(ofstream &oufd,const bool *charMap);

int writeBlkCharCodeTable(ofstream &oufd, char codeTable[][CODE_MAX_LEN]);

int writeBlkBwtIndex(ofstream &oufd,u32 );
int writeBlkZipNodeWithPreorder(ofstream &oufd, const waveletTree root);

#endif