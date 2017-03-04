#pragma once
#include "wzip.h"
#include "bitArray.h"
//for wavelet tree
//origBuffLen --> zipBuffLen    *2 for compress
//zipBuffLen  <-- blkSiz			for decompress
typedef struct waveletNode_t{
	u32		level;
	uchar	label;

	uchar*  bitBuff;
	u32		bitLen;// measure in bit

	
	//u16 HPheadLen;
	//uchar *HPhead;

	uchar*	zipBuff;
	u32		zipLen;// measure in bit

	waveletNode_t *leftChild;
	waveletNode_t *righChild;

	//for depress
	//u32 cursor;
	uchar *ptr;
	uchar offset;
}waveletNode_t;
typedef waveletNode_t * waveletTree;

waveletTree createWaveletTree(uchar *buff, u32 len,
	char(*codeTable)[CODE_MAX_LEN]
	);
int compressWaveletTree(waveletTree root, Stream_t &stream);
int computeZipSizWaveletTree(waveletTree root);
int destroyWaveletTree(waveletTree root);
int compressWaveletTreeWithDelta(waveletTree wavTree);