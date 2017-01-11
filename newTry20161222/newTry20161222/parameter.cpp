#include "parameter.h"
#include <string.h>
#include "wzip.h"
#include <stdlib.h>
/*
1-9		  1-3		1-2
wzip blkSiz100K treeType NodeType fileName

wzip -d fileName
*/

TreeType shapeMap(int val){
	switch (val)
	{
	case 1:
		return HUFFMAN;
		break;
	case 2:
		return BALANCE;
		break;
	case 3:
		return HU_TACKER;
		break;
	default:
		cout << "unknown shape,choose default" << endl;
		//printf("unknown shape,choose default\n");
		return BALANCE;

	}
}

NodeCodeType nodeCodeTypeMap(int val){
	switch (val)
	{
	case 1:
		return RLE_GAMA;
		break;
	case 2:
		return RLE_DELTA;
		break;
	case 3://----------baiwenwu-c----------
		return HBRID;
		break;
	case 4://----------baiwenwu-c----------
		return PLUSONE;
		break;
	default:
		cout << "unknown shape,choose default" << endl;
		//printf("unkonwn codeType, choose default\n");
		return RLE_GAMA;
	}
}

