#include"f_test.h"
#include "global.h"
int main()
{
	bool flag = true;
	for (int i = 1; i < 5; i++)
	{
		switch (i)
		{
		case 1:
			BitsCodeType = 1;
			cout << "RLE_GAMA ѹ����" << endl;
			break;
		/*case 2:
			BitsCodeType = 2;
			cout << "RLE_DELTA ѹ����" << endl;
			break;
		case 3:
			BitsCodeType = 4;
			cout << "PLUSONE ѹ����" << endl;
			break;
		case 4:
			BitsCodeType = 3;
			cout << "��ϱ��� ѹ����" << endl;
			break;*/
		default:
			flag = false;
			break;
		}
		getParameters();
		compressMainThread();
		int strLen = strlen((char*)FILENAME);
		char *src = (char*)FILENAME + strLen;
		strcpy_s(src, 10, ".wz");
		decompressMainThread();
		FILENAME[strLen] = '\0';

	}
	return 0;
}
