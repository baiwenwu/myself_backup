#include"f_test.h"
#include "global.h"
int main()
{
	getParameters();
	//compressMainThread();
	cout << "-------------decompress---------------" << endl;
	decompressMainThread();
	cout << "I love you!" << endl;
	return 0;
}
