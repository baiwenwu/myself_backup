#include "baseLib.h"
#include"bitArray.h"
#include "compress.h"


int main()
{
	fileStream fs;
	fileStream *pfs = &fs;
	pfs->srcNum = 1000;
	creatUcharArr(&pfs->src, pfs->srcNum + 1);
	randFillUcharArr(pfs->src, pfs->srcNum);
	//printString(pfs->src, pfs->srcNum + 1);
	//showGamma(pfs->src, pfs->srcNum*8);
	testCopyBitsFun(pfs->src, pfs->srcNum * 8);
	cout << endl;
	return 0;
}