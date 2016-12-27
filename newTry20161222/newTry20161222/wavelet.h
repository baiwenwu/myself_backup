#pragma once
#include "wzip.h"
//for wavelet tree
//origBuffLen --> zipBuffLen    *2 for compress
//zipBuffLen  <-- blkSiz			for decompress
typedef struct waveletNode_t{
	u32		level;
	uchar	label;

	uchar*  bitBuff;
	u32		bitLen;// measure in bit

	uchar*  head;
	u32        headNum;
	u32        headLen; //just for hybrid code

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