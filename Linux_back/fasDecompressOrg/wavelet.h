#ifndef _WAVELET_H
#define _WAVELET_H
#include "wzip.h"

waveletTree createWaveletTree(uchar *buff,u32 len,
								char (*codeTable)[CODE_MAX_LEN]
							 );
int destroyWaveletTree(waveletTree root);
//----------compress--------
int compressWaveletTree(waveletTree wavTree,NodeCodeType type,u32 HBblockSize);

int compressWaveletTreeWithGamma(waveletTree wavTree);

int compressWaveletTreeWithDelta(waveletTree wavTree);

int compressWaveletTreeWithHybird(waveletTree wavTree,u32 HBblockSize);

//----------decompress--------
int decompressWaveletTree(waveletTree wavTree,NodeCodeType type);

int decompressTreeWithRleGamma(waveletTree wavTree);

int decompressTreeWithRleDelta(waveletTree wavTree);

//int decompressWaveletTreeWithHybird(waveletTree wavTree,u32 HBblockSize);

int resetBitBuffWaveletTree(waveletTree root);

int resetZipBuffWaveletTree(waveletTree root);


int computeZipSizWaveletTree(waveletTree root);

#endif
