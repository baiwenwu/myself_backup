#include"bitMap.h"
void CreateBitMap()
{
	u16 R1 = 0,//打头的数字
		R2 = 0,//可解码个数
		R3 = 0,//偏移量
		R4 = 0;//有效解码位数,暂且不用
	u16 mask = -1;
	u32 tmp = -1;
	for (u32 i = 0; i < (1ULL << 8); i++)
	{
		u32 j = 1ULL << 15;
		u32 num0 = 0;
		while (j)
		{
			if (i&j)
				if (num0)
				{
					BitMap[i] = num0 << 24;
					break;
				}		
			num0++;
			j = j >> 1;
		}
	}
	for (u32 i = (1ULL << 8); i < 65536; i++)
	{
		R1 = R2 = R3 = R4 = 0;
		u32 andx = 1ULL << 15;
		u32 num0 = 0;
		while (andx)
		{
			if (i&andx)
			{
				if (!R2)
				{
					R2 = (num0 << 1) + 1;
					R1 = i >> (16 - R2);
				}
				if ((16 - R4)>(num0 << 1))
				{
					R3++;
					R4 += (num0 << 1) + 1;
				}
				andx = andx >> (num0+1);
				num0 = 0;
			}
			else
			{
				num0++;
				andx = andx >> 1;
			}
		}
		BitMap[i] += (R4 & 0xff);
		BitMap[i] = (BitMap[i] << 8) + (R3 & 0xff);
		BitMap[i] = (BitMap[i] << 8) + (R2 & 0xff);
		BitMap[i] = (BitMap[i] << 8) + (R1 & 0xff);
	}
	//for (u32 i = (1ULL << 8); i<65536; i++)
	//{
	//	R1 = R2 = 0;
	//	int j = 15;
	//	u32 num0 = 0;
	//	while (j >= 0)
	//	{
	//		if (i&(1ULL << j))
	//		{
	//			j -= num0;
	//			if (j >= 0)
	//			{
	//				//if (j+num0+num0==15)
	//				if (!R2)
	//				{
	//					R2 = (num0 << 1) + 1;
	//					R1 = i >> (16 - R2);
	//				}
	//				R3 ++;
	//				R4 = 16 - j;
	//				num0 = 0;
	//			}
	//			else
	//				break;

	//		}
	//		else
	//		{
	//			num0++;
	//		}
	//		j--;
	//	}
	//	/*cout << i;
	//	cout << "\t" << bitset<16>(i);
	//	cout << "   R4:" << R4;
	//	cout << "   R3:" << R3;
	//	cout << "   R2:" << R2;
	//	cout << "   R1:" << R1;*/
	//	BitMap[i] += (R1 & 0xff);
	//	BitMap[i] = (BitMap[i] << 8) + (R2 & 0xff);
	//	BitMap[i] = (BitMap[i] << 8) + (R3 & 0xff);
	//	//cout << "  " << BitMap[i] << endl;
	//}
}
void printBitMap()
{
	u16 R1 = 0,//打头的数字
		R2 = 0,//可解码个数
		R3 = 0,//偏移量
		R4 = 0;//有效解码位数,暂且不用
	for (u32 i = (1ULL << 8); i < 65536; i++)
	{
		cout << "\t" << bitset<16>(i);
		R1 = BitMap[i] & 0xff;
		R2 = (BitMap[i] >> 8) & 0xff;
		R3 = (BitMap[i] >> 16) & 0xff;
		R4 = (BitMap[i] >> 24) & 0xff;
		cout << "   R4:" << R4;
		cout << "   R3:" << R3;
		cout << "   R2:" << R2;
		cout << "   R1:" << R1;
		cout << endl;
	}
}