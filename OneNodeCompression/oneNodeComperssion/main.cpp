#include "baseLib.h"
#include"bitArray.h"
#include "compress.h"
#include"decompress.h"


int main()
{
	fileStream fs;
	fileStream *pfs = &fs;
	pfs->srcNum = 1000;
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
	cout << pfs->cdLen << endl;
	//derunLengthHybirdCode(pfs);
	CreateBitMap();
	printBitMap();
	return 0;
}