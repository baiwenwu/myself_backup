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
	pfs->srcLen = pfs->srcNum << 3;
	pfs->cdNum = pfs->cdNum;
	runLengthHybirdCode(pfs);
	cout << endl;
	//--------------解压测试-----------
	derunLengthHybirdCode(pfs);
	writeSrcFile(pfs->src, pfs->srcNum);
	writeToFile(pfs);
	fileStream tmp;
	readFromFile(&tmp);
	if (isEqual(&tmp, pfs))
	{
		cout << "good,读出的结果和写入的结果一致！" << endl;
	}
	else
	{
		cout << "bad,读出的结果和写入的结果不一致！" << endl;
	}
	return 0;
}