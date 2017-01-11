#include"f_test.h"
#include "global.h"
int main()
{
	for (int i = 1; i < 4; i++)
	{
		switch (i)
		{
		case 1:
			BitsCodeType = 1;
			cout << "RLE_GAMA Ñ¹Ëõ£º" << endl;
			break;
		case 2:
			BitsCodeType = 2;
			cout << "RLE_DELTA Ñ¹Ëõ£º" << endl;
			break;
		case 3:
			BitsCodeType = 4;
			cout << "PLUSONE Ñ¹Ëõ£º" << endl;
			break;
		default:
			break;
		}
		//BitsCodeType = HBRID;
		getParameters();
		compressMainThread();
		int strLen = strlen((char*)FILENAME);
		char *src = (char*)FILENAME + strLen;
		strcpy_s(src, 10, ".wz");
		decompressMainThread();
		FILENAME[strLen] = '\0';

	}
	cout << "I love you!" << endl;
	return 0;
}
