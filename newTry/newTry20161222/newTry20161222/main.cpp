#include"f_test.h"
#include "global.h"
#include<vector>
#if 0
int main_t()
{
	bool flag = true;
	for (int i = 1; i < 5; i++)
	{
		switch (i)
		{
		case 1:
			BitsCodeType = 1;
			cout << "RLE_GAMA 压缩：" << endl;
			break;
		case 2:
			BitsCodeType = 2;
			cout << "RLE_DELTA 压缩：" << endl;
			break;
		case 3:
			BitsCodeType = 4;
			cout << "PLUSONE 压缩：" << endl;
			break;
		/*case 4:
			BitsCodeType = 3;
			cout << "混合编码 压缩：" << endl;
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
#endif

int main()
{
	double time1 = 0;
	double counts = 0;
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nBeginTime;
	LARGE_INTEGER nEndTime;
	bool flag = true;
	vector<double> gammaTime;
	vector<double> deltaTime;
	vector<double> plusOneTime;
	int num = 20;
	while (num--)
	{
		for (int i = 1; i < 5; i++)
		{
			//i = 3;
			switch (i)
			{
			case 1:
				BitsCodeType = 1;
				cout << "RLE_GAMA 压缩：" << endl;
				break;
			case 2:
				BitsCodeType = 2;
				cout << "RLE_DELTA 压缩：" << endl;
				break;
			case 3:
				BitsCodeType = 4;
				cout << "PLUSONE 压缩：" << endl;
				break;
				/*case 4:
				BitsCodeType = 3;
				cout << "混合编码 压缩：" << endl;
				break;*/
			default:
				flag = false;
				//i = 1;
				break;
			}
			if (flag)
			{
				QueryPerformanceFrequency(&nFreq);
				QueryPerformanceCounter(&nBeginTime);//开始计时 
				getParameters();
				compressMainThread();
				int strLen = strlen((char*)FILENAME);
				char *src = (char*)FILENAME + strLen;
				strcpy_s(src, 10, ".wz");
				decompressMainThread();

				QueryPerformanceCounter(&nEndTime);//停止计时 
				time1 = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s 
				cout << "程序执行时间：" << time1 * 1000 << "ms" << endl;
				switch (i)
				{
				case 1:
					gammaTime.push_back(time1 * 1000);
					break;
				case 2:
					deltaTime.push_back(time1 * 1000);
					break;
				case 3:
					plusOneTime.push_back(time1 * 1000);
					break;
					/*case 4:
					BitsCodeType = 3;
					cout << "混合编码 压缩：" << endl;
					break;*/
				default:
					flag = false;
					//i = 1;
					break;
				}

				FILENAME[strLen] = '\0';
				cout << "_______________" << num << "__________________" << endl;

			}
			flag = true;
		}
	}
	cout << gammaTime.size() << endl;
	double averageTime = 0;
	ofstream out;
	out.open("加速写入.txt", ios::out);
	if (!out.is_open())
	{
		cout << "open file error!" << endl;
	}
	out << "gammaTime:" << endl;
	for (int i = 0; i < gammaTime.size(); i++)
	{
		averageTime += gammaTime[i];
		out << gammaTime[i] << " ";
	}
	averageTime = averageTime / gammaTime.size();
	out << "\naverageTime: " << averageTime << endl;

	averageTime = 0;
	out << "\ndeltaTime:" << endl;
	for (int i = 0; i < deltaTime.size(); i++)
	{
		averageTime += deltaTime[i];
		out << deltaTime[i] << " ";
	}
	averageTime = averageTime / deltaTime.size();
	out << "\naverageTime: " << averageTime << endl;

	averageTime = 0;
	out << "\nplusOneTime: " << endl;
	for (int i = 0; i < plusOneTime.size(); i++)
	{
		averageTime += plusOneTime[i];
		out << plusOneTime[i] << " ";
	}
	averageTime = averageTime / plusOneTime.size();
	out << "\naverageTime: " << averageTime << endl;
	out << endl;
	out.close();
	return 0;
}
