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
	//--------------��ѹ����-----------
	derunLengthHybirdCode(pfs);
	writeSrcFile(pfs->src, pfs->srcNum);
	writeToFile(pfs);
	fileStream tmp;
	readFromFile(&tmp);
	if (isEqual(&tmp, pfs))
	{
		cout << "good,�����Ľ����д��Ľ��һ�£�" << endl;
	}
	else
	{
		cout << "bad,�����Ľ����д��Ľ����һ�£�" << endl;
	}
	return 0;
}