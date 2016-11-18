#ifndef STATIC_H
#define STATIC_H

#include <stdint.h>
#include <vector>
#include <math.h>
#include <string.h>
#include <stdlib.h>

class Statics{

public:
	static int32_t fileSize_kkz;//数据文件大小
	static int32_t block_size;
	static int32_t superblock_size;
	static int32_t coding_styles[5];
	static int32_t *runs_num;
	static int64_t N;
	static double aveRun;

	static double aveGamma;
	static double aveDelta;
	static int32_t numOfRlg0_1;
	static double aveGammaOfBlock;
	static double aveDeltaOfBlock;

	static void init();

	static inline int32_t floor_log2(uint64_t i){
		return floor(log(i) / log(2));
	}

	//runs in blocks
	static int32_t *runs_blocks;
	static int64_t M;
	static double aveRunBlock;
	//size of everyPart
	static double headerSize;
	static double SSize;
	static double plainSize;
	//static double SBrankSize;
	//static double BrankSize;
	//SBSize(size of rankSuperblock + size of offsetSuperblock)
	static double SBSize;
	//SBsize(size of rankBlock +size of offsetBlock)
	static double BSize;

	// runs of BWT(L)
	static double aveRunsOfL;


	static  char* getFile(const char* filename){
		char* file1 = new char[256];
		strcpy(file1, filename);
		int i , j;
		int len = strlen(filename);
		for(i = len; i >= 0; i--){
			if(file1[i] == '/')
				break;
		}
		if(i != -1){
			for(j = 0, i++; i < len; i++, j++)
				file1[j] = file1[i];
			file1[j] = '\0';
		}
		return file1;	
	}

};



#endif