#include "rlElisCode.h"
#include "wzip.h"
# include <memory.h>
#include <math.h>
void printBitsForArray(uchar * src, u32 index, u32 len)
{
	u32 words = index >> 3;
	u32 offset = index & 0x7;
	u32 i = 0;
	uchar andx = 128 >> offset;
	int num = 0;
	while (i < len)
	{
		while (andx && i<len)
		{
			if (andx&src[words])
			{
				printf("1");
			}
			else
			{
				printf("0");
			}
			i++;
			if (!(i & 7))
			{
				num++;
				printf(" ");
				if ((num / 8) && (num % 8 == 0))
				{
					cout << " (" << num / 8 << ")" << endl;
				}
			}
			offset++;
			andx = andx >> 1;
		}
		andx = 128;
		if (!(offset & 7))
		{
			offset = 0;
			words++;
		}
	}
}
int bitsNumTbl[256] = {//plusOne与gamma差值计算加速表
	-1, 0, 1, 1, 2, 2, 2, 2,3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,	7, 7, 7, 7, 7, 7, 7, 7};
int getBitsNum(unsigned x)
{
	if (x<256){
		return bitsNumTbl[x];
	}
	int n = 1;
	if (x == 0) return -1;
	if ((x >> 16) == 0) { n = n + 16; x = x << 16; }
	if ((x >> 24) == 0) { n = n + 8; x = x << 8; }
	if ((x >> 28) == 0) { n = n + 4; x = x << 4; }
	if ((x >> 30) == 0) { n = n + 2; x = x << 2; }
	n = n - (x >> 31);
	return 31 - n;
}
int DiffGPTbls[512] = {//gamma-plusOne
	-1, -1, 1, -2, 0, 0, 0, -1, 1, 1, 1, 1, 1, 1, 1, -2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};
int getSubGP(u32 x)
{
	if (x<512){
		return DiffGPTbls[x];
	}
	else
	{
		int numGa = (getBitsNum(x) << 1) + 1;
		int numDe = (getBitsNum(getBitsNum(x + 1)) << 1) + getBitsNum(x + 1) + 1;
		return numGa - numDe;
	}
}
uchar acRunsTbl[256] = {//runs加速表
	8,
	7,
	6, 6,
	5, 5, 5, 5,
	4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5,
	6, 6,
	7,
	8
};

u16 degaTab[512] = {//gamma 加速表
	2304, 2048, 1792, 1792, 1536, 1536, 1536, 1536, 1280, 1280, 1280, 1280, 1280, 1280, 1280, 1280,
	2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 2328, 2329, 2330, 2331, 2332, 2333, 2334, 2335,
	1800, 1800, 1800, 1800, 1801, 1801, 1801, 1801, 1802, 1802, 1802, 1802, 1803, 1803, 1803, 1803,
	1804, 1804, 1804, 1804, 1805, 1805, 1805, 1805, 1806, 1806, 1806, 1806, 1807, 1807, 1807, 1807,
	1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284, 1284,
	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,
	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,
	1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770, 770,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771, 771,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257, 257
};
u16 dedeTab[512] = {//delta加速表
	2304, 2048, 1792, 1792, 1536, 1536, 1536, 1536, 1280, 1280, 1280, 1280, 1280, 1280, 1280, 1280,
	2319, 2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 2328, 2329, 2330, 2331, 2332, 2333, 2334,
	1799, 1799, 1799, 1799, 1800, 1800, 1800, 1800, 1801, 1801, 1801, 1801, 1802, 1802, 1802, 1802,
	1803, 1803, 1803, 1803, 1804, 1804, 1804, 1804, 1805, 1805, 1805, 1805, 1806, 1806, 1806, 1806,
	2088, 2088, 2089, 2089, 2090, 2090, 2091, 2091, 2092, 2092, 2093, 2093, 2094, 2094, 2095, 2095,
	2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359, 2360, 2361, 2362, 2363, 2364, 2365, 2366, 2367,
	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,
	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,
	1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058,
	1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058, 1058,
	1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059,
	1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059, 1059,
	1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316,
	1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317,
	1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318,
	1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289,
	289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289, 289
};
u16 plusOlen[512] = {//plusOne加速表
	4864, 4352, 3840, 3840, 3328, 3328, 3328, 3328, 2816, 2816, 2816, 2816, 2816, 2816, 2816, 2816,
	2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 2328, 2329, 2330, 2331, 2332, 2333, 2334, 2335,
	1800, 1800, 1800, 1800, 1801, 1801, 1801, 1801, 1802, 1802, 1802, 1802, 1803, 1803, 1803, 1803,
	1804, 1804, 1804, 1804, 1805, 1805, 1805, 1805, 1806, 1806, 1806, 1806, 1807, 1807, 1807, 1807,
	2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359, 2360, 2361, 2362, 2363, 2364, 2365, 2366, 2367,
	1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285, 1285,
	1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286, 1286,
	1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287, 1287,
	1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316, 1316,
	1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317, 1317,
	1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318, 1318,
	1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319, 1319,
	1576, 1576, 1576, 1576, 1576, 1576, 1576, 1576, 1577, 1577, 1577, 1577, 1577, 1577, 1577, 1577,
	1578, 1578, 1578, 1578, 1578, 1578, 1578, 1578, 1579, 1579, 1579, 1579, 1579, 1579, 1579, 1579,
	1580, 1580, 1580, 1580, 1580, 1580, 1580, 1580, 1581, 1581, 1581, 1581, 1581, 1581, 1581, 1581,
	1582, 1582, 1582, 1582, 1582, 1582, 1582, 1582, 1583, 1583, 1583, 1583, 1583, 1583, 1583, 1583,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546, 546,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547,
	547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547, 547
};


uchar *EndWords = NULL;
uchar EndOff = 0;
u32 getRuns(uchar **src, uchar *offset)
{
	uchar *ptr = *src;
	uchar off_t = *offset;
	uchar ch_t = *ptr << off_t;
	u32 runs = acRunsTbl[ch_t];
	if (runs < 8 - off_t)
	{
		*offset += runs;
		return runs;
	}
	runs = 8 - off_t;
	while (++ptr < EndWords){
		uchar ch_t1 = *ptr >> 7;
		uchar ch_t2 = *(ptr - 1) & 1;
		if ((*ptr >> 7) == ((*(ptr - 1)) & 1))
		{
			uchar runs_t = acRunsTbl[*ptr];
			runs += runs_t;
			if (runs_t < 8)
			{
				*src = ptr;
				*offset = runs_t;
				return runs;
			}
		}
		else{
			*src = ptr;
			*offset = 0;
			return runs;
		}
	}
	runs = (EndWords - *src) * 8 + EndOff - *offset;
	*src = EndWords;
	*offset = EndOff;
	return EndOff ? runs - 8 : runs;
}
void writeRuns_t(uchar *src, uchar offset, u32 runs)
{
	uchar *src_t = src + ((offset + runs) >> 3);
	uchar src_off = (offset + runs) & 0x7;
	*src += 0xff >> offset;
	while (src < src_t)
	{
		*(++src) = 0xff;
	}
	*src = *src&(0xff << (8 - src_off));
}
u32 getBitsPO1(uchar *src, uchar srcOff, u16 len)
{
	u32 val = *src++;
	val = (val << 8) + *src++;
	if (len < 16 - srcOff)
	{
		val = val << (16 + srcOff);
		return val >> (32 - len);
	}
	val = (val << 8) + *src++;
	if (len < 24 - srcOff)
	{
		val = val << (8 + srcOff);
		return val >> (32 - len);
	}
	val = (val << 8) + *src++;
	if (len < 32 - srcOff)
	{
		val = val << srcOff;
		return val >> (32 - len);
	}
}
uchar getRunsForDeg(uchar **src, uchar *srcOff)
{
	uchar Runs = 8;
	uchar *src_t = *src + 1;
	while (*src_t == 0)
	{
		src_t++;
		Runs += 8;
	}
	uchar srcOff_t = *srcOff;
	*srcOff = (*src_t) & 128 ? 0 : acRunsTbl[*src_t];
	*src = src_t;
	return Runs + *srcOff - srcOff_t;
}
//----------------gamma--------------------
void Append_g(u32 runs, uchar **buffPPtr, uchar *offset)
{//runlength gamma
	uchar *ptr = *buffPPtr;
	uchar *ptrs = ptr;
	uchar off = *offset;
	uchar bitsLen = (getBitsNum(runs) << 1) + 1;
	ptr += (off + bitsLen) >> 3;
	*buffPPtr = ptr;
	off = (off + bitsLen) & 0x7;
	*offset = off;
	runs = runs << (8 - off);
	*ptr += runs & 0xff;
	if (ptr == ptrs)
		return;
	*(--ptr) += (runs >> 8) & 0xff;
	if (ptr == ptrs)
		return;
	*(--ptr) += (runs >> 16) & 0xff;
	if (ptr == ptrs)
		return;
	*(--ptr) += (runs >> 24) & 0xff;
	if (ptr == ptrs)
		return;
}
int runLengthGammaCode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}
	uchar *savedDst = dst;
	uchar *src_t = src;
	uchar srcOff = 0;
	u32 srcNum = (bitsLen >> 3) + ((bitsLen & 0x7) ? 1 : 0);
	EndWords = src + srcNum;
	EndOff = bitsLen & 0x7;

	//get fist bit of src
	*dst = *src & 128;
	uchar offset = 1;
	bool flag;
	if (*dst)
		flag = true;
	else
		flag = false;
	u32 runs;
	while (src_t<EndWords)
	{
		runs = getRuns(&src_t, &srcOff);
		//elisGammaCode(runs, &dst, &offset);
		if (runs == 0)break;
		Append_g(runs, &dst, &offset);
	}
	uchar srcOff_t = srcOff;
	while (srcOff_t < EndOff)
	{
		runs = getRuns(&src_t, &srcOff);
		srcOff_t += runs;
		//elisGammaCode(runs, &dst, &offset);
		Append_g(runs, &dst, &offset);
	}
	return (dst - savedDst) * 8 + offset;
}
int elisGammaCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	int ret;
	if (!num || !buffPPtr || !offset)
	{
		return ERR_PARAMETER;
	}
	uchar *ptr = *buffPPtr;
	uchar off = *offset;

	u32 bitsLen = getBitsNum(num);
	if (bitsLen == -1)
	{
		return -1;
	}

	u32 i;

	//can't mark off this segments
	for (i = 0; i<bitsLen; i++)
	{
		*ptr &= ~(1 << (7 - off));
		off++;
		if (off == 8)
		{
			off = 0;
			ptr++;
		}
	}
	*ptr |= 1 << (7 - off);
	if (++off == 8)
	{
		off = 0;
		ptr++;
	}

	for (i = 1; i <= bitsLen; i++)
	{
		if (num & (1 << (bitsLen - i)))
		{
			*ptr |= (1 << (7 - off));
		}
		else
		{
			*ptr &= ~(1 << (7 - off));
		}

		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
	}
	*buffPPtr = ptr;
	*offset = off;
	return 0;

}
int runLengthGammaDecode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}
	uchar *savedSrc = src;
	uchar *savedDst = dst;

	bool flag = src[0]&128 ? true : false;

	uchar srcOffset = 1;
	uchar dstOffset = 0;

	u32 Runs, deLen = 1;
	uchar R1, R2, Rt;
	while (deLen < bitsLen)
	{
		u16 val = *src;
		val = (val << 8) | src[1];
		val = val << srcOffset;
		u16 val_t = val >> 7;
		R2 = degaTab[val_t] >> 8;
		Runs = degaTab[val_t] & 0xff;
		if (Runs)
		{
			deLen += R2;
			if (flag)
				writeRuns_t(dst, dstOffset, Runs);
		}
		else
		{
			if (R2 < 9){
				src += (srcOffset + R2) >> 3;
				srcOffset = (srcOffset + R2) & 0x7;
				R2++;
				Runs = getBitsPO1(src, srcOffset, R2);
			}
			else{
				R2 = getRunsForDeg(&src, &srcOffset);
				R2++;
				Runs = getBitsPO1(src, srcOffset, R2);
			}
			if (flag)
				writeRuns_t(dst, dstOffset, Runs);
			deLen += (R2 << 1) - 1;
		}
		flag = !flag;
		dst += (dstOffset + Runs) >> 3;
		dstOffset = (dstOffset + Runs) & 0x7;
		src += (srcOffset + R2) >> 3;
		srcOffset = (srcOffset + R2) & 0x7;
	}
	return (dst - savedDst) * 8 + dstOffset;
}

//----------------delta--------------------
int elisDeltaCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	if (!num || !buffPPtr || !offset)
	{
		return ERR_PARAMETER;
	}

	u32 bitsLen = getBitsNum(num);
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	elisGammaCode(bitsLen + 1, &ptr, &off);

	u32 i;
	for (i = 1; i <= bitsLen; i++)
	{
		if (num &(1 << (bitsLen - i)))
		{
			*ptr |= 1 << (7 - off);
		}
		else
		{
			*ptr &= ~(1 << (7 - off));
		}
		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
	}

	*buffPPtr = ptr;
	*offset = off;
	return 0;
}
int runLengthDeltaCode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}
	u32 srcNum = (bitsLen >> 3) + ((bitsLen & 0x7) ? 1 : 0);
	EndWords = src + srcNum;
	EndOff = bitsLen & 0x7;

	uchar *savedDst = dst;
	uchar dstOffset = 1;

	uchar *src_t = src;
	uchar srcOff = 0;
	
	//get fist bit of src
	*dst = *src & 128;

	bool flag = *dst ? true : false;
	u32 runs;
	while (src_t<EndWords)
	{
		runs = getRuns(&src_t, &srcOff);
		elisDeltaCode(runs, &dst, &dstOffset);
	}
	uchar srcOff_t = srcOff;
	while (srcOff_t < EndOff)
	{
		runs = getRuns(&src_t, &srcOff);
		srcOff_t += runs;
		elisDeltaCode(runs, &dst, &dstOffset);
	}
	return (dst - savedDst) * 8 + dstOffset;
}
int runLengthDeltaDecode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}
	uchar srcOffset = 1;
	uchar dstOffset = 0;

	uchar *savedDst = dst;
	uchar *savedSrc = src;

	//get the first bit
	bool flag = *src & 128 ? 1 : 0;
	u32 Runs, deLen = 1;
	u32 R1, R2;
	while (deLen < bitsLen)
	{
		Runs = 1;
		u16 val = *src;
		val = (val << 8) | src[1];
		val = val << srcOffset;
		u16 poVal = val >> 7;
		R2 = (dedeTab[poVal] >> 8) & 0xff;
		R1 = dedeTab[poVal] & 0xff;
		if (R1 < 32){
			Runs = Runs << R1;
			src += (srcOffset + R2) >> 3;
			srcOffset = (srcOffset + R2) & 0x7;
			Runs += getBitsPO1(src, srcOffset, R1);
			src += (srcOffset + R1) >> 3;
			srcOffset = (srcOffset + R1) & 0x7;
			deLen += R1 + R2;
		}
		else
		{
			Runs = R1 - 32;
			src += (srcOffset + R2) >> 3;
			srcOffset = (srcOffset + R2) & 0x7;
			deLen += R2;
		}
		if (flag)
		{
			writeRuns_t(dst, dstOffset, Runs);
		}
		dst = dst + ((dstOffset + Runs) >> 3);
		dstOffset = (dstOffset + Runs) & 7;
		flag = !flag;
	}
	return (dst - savedDst) * 8 + dstOffset;
}

//----------------plusOne--------------------
void writePlusOne(u32 num, uchar **buffPPtr, uchar *offset)
{
	num = num + 1;//plusOne
	u32 bitsLen = getBitsNum(num);
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	Append_g(bitsLen, &ptr, &off);
	num = num << (32 - bitsLen);
	num = num >> off;
	*buffPPtr = ptr + ((off + bitsLen) >> 3);
	*offset = (off + bitsLen) & 0x7;
	*(ptr++) += (num >> 24) & 0xff;
	if (bitsLen < 16 - off)
	{
		*ptr += (num >> 16) & 0xff;
		return;
	}
	if (bitsLen < 24 - off)
	{
		*(ptr++) += (num >> 16) & 0xff;
		*ptr += (num >> 8) & 0xff;
		return;
	}
	if (bitsLen <= 32)
	{
		*(ptr++) += (num >> 16) & 0xff;
		*(ptr++) += (num >> 8) & 0xff;
		*ptr += num & 0xff;
		return;
	}
}
int plusOneCode(u32 num, uchar **buffPPtr, uchar *offset)
{
	if (!num || !buffPPtr || !offset)
	{
		return -1;
	}
	num = num + 1;
	u32 bitsLen = getBitsNum(num);
	uchar *ptr = *buffPPtr;
	uchar off = *offset;
	elisGammaCode(bitsLen, &ptr, &off);

	u32 i;
	for (i = 1; i <= bitsLen; i++)
	{
		if (num &(1 << (bitsLen - i)))
		{
			*ptr |= 1 << (7 - off);
		}
		else
		{
			*ptr &= ~(1 << (7 - off));
		}
		if (++off == 8)
		{
			off = 0;
			ptr++;
		}
	}

	*buffPPtr = ptr;
	*offset = off;
	return 0;
}
int runLengthPlusOneCode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}
	uchar *savedDst = dst;
	uchar dstOffset = 1;

	uchar *src_t = src;
	uchar srcOff = 0;
	u32 srcNum = (bitsLen >> 3) + ((bitsLen & 0x7) ? 1 : 0);
	EndWords = src + srcNum;
	EndOff = bitsLen & 0x7;

	//get fist bit of src
	*dst = *src & 128;
	bool flag;
	if (*dst)
		flag = true;
	else
		flag = false;
	u32 runs;
	while (src_t<EndWords)
	{
		runs = getRuns(&src_t, &srcOff);
		plusOneCode(runs, &dst, &dstOffset);
		//writePlusOne(runs, &dst, &dstOffset);
	}
	uchar srcOff_t = srcOff;
	while (srcOff_t < EndOff)
	{
		runs = getRuns(&src_t, &srcOff);
		srcOff_t += runs;
		plusOneCode(runs, &dst, &dstOffset);
		//writePlusOne(runs, &dst, &dstOffset);
	}
	return (dst - savedDst) * 8 + dstOffset;
}
int runLengthPlusOneDecode(uchar *src, u32 bitsLen, uchar *dst)
{
	if (!src || !bitsLen || !dst)
	{
		return ERR_PARAMETER;
	}
	uchar srcOffset = 1;
	uchar dstOffset = 0;

	uchar *savedDst = dst;
	uchar *savedSrc = src;

	//get the first bit
	bool flag = *src & 128 ? 1 : 0;
	u32 Runs, deLen = 1;
	u32 R1, R2;
	while (deLen < bitsLen)
	{
		Runs = 1;
		u16 val = *src;
		val = (val << 8) | src[1];
		val = val << srcOffset;
		u16 poVal = val >> 7;
		R2 = (plusOlen[poVal] >> 8) & 0xff;
		R1 = plusOlen[poVal] & 0xff;
		if (R1 < 32){
			Runs = Runs << R1;
			src += (srcOffset + R2) >> 3;
			srcOffset = (srcOffset + R2) & 0x7;
			Runs += getBitsPO1(src, srcOffset, R1);
			src += ((srcOffset + R1) >> 3);
			srcOffset = (srcOffset + R1) & 0x7;
			deLen += R1 + R2;
			Runs--;
		}
		else
		{
			Runs = R1 - 33;
			src += (srcOffset + R2) >> 3;
			srcOffset = (srcOffset + R2) & 0x7;
			deLen += R2;
		}
		if (flag)
		{
			writeRuns_t(dst, dstOffset, Runs);
		}
		dst = dst + ((dstOffset + Runs) >> 3);
		dstOffset = (dstOffset + Runs) & 7;
		flag = !flag;
	}
	return (dst - savedDst) * 8 + dstOffset;
}

//----------------hybird--------------------
uchar getMark2(uchar **src, uchar *offset)
{
	uchar *ptr = *src;
	uchar offset_t = *offset;
	if ((offset_t + 2) & 0x8)
	{//>8
		u16 val_t = *(ptr++);
		val_t = (val_t << 8) + *ptr;
		val_t = val_t << offset_t;
		*offset = (*offset + 2) & 0x7;
		*src = ptr;
		return (val_t >> (14)) & 0xff;
	}
	uchar ch_t = *ptr << offset_t;
	*offset += 2;
	*src = ptr;
	return ch_t >> 6;
}
void writeMark2(uchar **src, uchar *offset, uchar val)
{
	uchar *ptr = *src;
	uchar offset_t = *offset;
	if ((offset_t + 2) & 0x8)
	{
		u16 val_t = *ptr;
		val_t = (val_t << (offset_t - 6)) + val;
		val_t = val_t << (14 - offset_t);
		*(ptr++) = val_t >> 8;
		*ptr = val_t & 0xff;
		*offset = (*offset + 2) & 0x7;
		*src = ptr;
		return;
	}
	*ptr += val << (6 - offset_t);
	*src = ptr;
	*offset += 2;
}
void bitsCopy(uchar **dst, uchar *dstOff, uchar **src, uchar *srcOff, u32 len)
{
	uchar *dstPtr = *dst;
	uchar dstOff_t = *dstOff;
	uchar *srcPtr = *src;
	uchar srcOff_t = *srcOff;
	uchar *endDstWord = dstPtr + ((len + dstOff_t) >> 3);
	uchar dstoff_t = (len + dstOff_t) & 0x7;
	*dst = endDstWord;
	*dstOff = dstoff_t;
	*src = srcPtr + ((len + srcOff_t) >> 3);
	*srcOff = (len + srcOff_t) & 0x7;
	if (srcOff_t == dstOff_t)
	{
		uchar ch_t = *(srcPtr++) << srcOff_t;
		*(dstPtr++) += (ch_t >> dstOff_t);
		while (dstPtr < endDstWord)*(dstPtr++) = *(srcPtr++);
		*dstPtr = *srcPtr >> (8 - dstoff_t);
		*dstPtr = *dstPtr << (8 - dstoff_t);
		return;
	}
	uchar offset;
	uchar ch_t;
	if (srcOff_t > dstOff_t)
	{
		ch_t = *srcPtr << srcOff_t;
		*dstPtr += (ch_t >> dstOff_t);
		offset = 8 - (srcOff_t - dstOff_t);
	}
	else
	{
		ch_t = *srcPtr << srcOff_t;
		*(dstPtr++) += (ch_t >> dstOff_t);
		offset = dstOff_t - srcOff_t;
		*dstPtr += (ch_t << (8 - dstOff_t));
	}
	while (dstPtr < endDstWord)
	{
		uchar ch_t = *(++srcPtr);
		*(dstPtr++) += (ch_t >> offset);
		*dstPtr = (ch_t << (8 - offset));
	}
	ch_t = *(++srcPtr);
	*dstPtr += (ch_t >> offset);
	*dstPtr = *dstPtr >> (8 - dstoff_t);
	*dstPtr = *dstPtr << (8 - dstoff_t);
}
u32 *Runs = NULL;
int gppHybirdCode(uchar *src, u32 bitsLen, uchar *dst, u16 HBblSize)
{
	int ret;
	if (!src || !bitsLen || !dst)
	{
		return -1;
	}
	uchar *savedDst = dst;
	uchar *savedSrc = src;
	u32 srcNum = (bitsLen >> 3) + ((bitsLen & 0x7) ? 1 : 0);
	EndWords = src + srcNum;
	EndOff = bitsLen & 0x7;
	//get fist bit of src
	*dst = *src&(0x80);//将第一个bit写入压缩串
	bool flag = *dst ? 1 : 0;
	uchar dstOffset = 1;
	uchar srcOffset = 0;
	//记录上一个块结束位置
	uchar *lastSrcWord;
	uchar lastSrcOff;
	//Runs函数
	int i;
	while (src < EndWords)
	{
		u32 k = 0;
		u32 rbits = 0;
		int gapBits = 0;
		u32 sumBits = 0;
		lastSrcWord = src;
		lastSrcOff = srcOffset;
		while (rbits < HBblSize&&src < EndWords)
		{
			Runs[k] = getRuns(&src, &srcOffset);
			if (Runs[k] == 0)break;
			rbits += Runs[k];
			gapBits += getSubGP(Runs[k++]);
			flag = !flag;
		}
		if (k == 0)
			break;
		if (k == 1 && HBblSize<rbits)
		{//plusOne is good
			writeMark2(&dst, &dstOffset, 3);//plusOne
			writePlusOne(Runs[0], &dst, &dstOffset);
			continue;
		}
		for (i = 0; i < k; i++)//可以k-1，用来调试块大小
			sumBits += getBitsNum(Runs[i]);
		sumBits = (sumBits << 1) + k;
		if (src < EndWords)
		{
			if (sumBits <= rbits)
			{
				if (gapBits <= 0)
				{
					writeMark2(&dst, &dstOffset, 2);//gamma
					for (i = 0; i < k; i++)
						Append_g(Runs[i], &dst, &dstOffset);
					continue;
				}
				writeMark2(&dst, &dstOffset, 3);//plusOne
				for (i = 0; i < k; i++)
					writePlusOne(Runs[i], &dst, &dstOffset);
				continue;
			}
			//调整flag
			if (rbits > HBblSize)
			{
				flag = !flag;//调整runs
				//u32 bits_t = sumBits - HBblSize;	
			}
			if (flag)
				writeMark2(&dst, &dstOffset, 1);//plain1
			else
				writeMark2(&dst, &dstOffset, 0);//plain0
			src = lastSrcWord;
			srcOffset = lastSrcOff;
			bitsCopy(&dst, &dstOffset, &src, &srcOffset, HBblSize);
			continue;
		}
		if (sumBits <= rbits + 7)
		{
			if (gapBits <= 0)
			{
				writeMark2(&dst, &dstOffset, 2);//gamma
				for (i = 0; i < k; i++)
					Append_g(Runs[i], &dst, &dstOffset);
				continue;
			}
			writeMark2(&dst, &dstOffset, 3);//plusOne
			for (i = 0; i < k; i++)
				writePlusOne(Runs[i], &dst, &dstOffset);
			continue;
		}
		else{//这种情况应该极少出现
			writeMark2(&dst, &dstOffset, 0);//plain0
			src = lastSrcWord;
			srcOffset = lastSrcOff;
			if (rbits>HBblSize)rbits = HBblSize;
			bitsCopy(&dst, &dstOffset, &src, &srcOffset, rbits);
		}
	}
	if (srcOffset < EndOff)
	{
		int xxxx = 0;
	}
	return (dst - savedDst) * 8 + dstOffset;
}