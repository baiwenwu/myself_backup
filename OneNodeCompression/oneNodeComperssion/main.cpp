#include "baseLib.h"
#include"bitArray.h"
#include "compress.h"
#include"decompress.h"


int main()
{
	CreateBitMap();
	fileStream fs;
	fileStream *pfs = &fs;
	pfs->srcNum = 10000;
	creatUcharArr(&pfs->src, pfs->srcNum + 1);
	randFillUcharArr(pfs->src, pfs->srcNum);
	/*printString(pfs->src, pfs->srcNum + 1);
	showGamma(pfs->src, pfs->srcNum*8);
	testCopyBitsFun(pfs->src, pfs->srcNum * 8);*/
	pfs->srcLen = pfs->srcNum << 3;
	pfs->cdNum = pfs->cdNum;
	runLengthHybirdCode(pfs);
	cout << endl;
	//--------------½âÑ¹²âÊÔ-----------
	derunLengthHybirdCode(pfs);
	return 0;
}