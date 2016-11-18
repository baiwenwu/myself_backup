#include "statics.h"
int32_t Statics::fileSize_kkz=0;
int32_t Statics::block_size=0;
int32_t Statics::superblock_size=0;
int32_t Statics::coding_styles[5] = {0};
int64_t Statics::N = 1024*1024*600;
int32_t* Statics::runs_num = new int32_t[N];
double Statics::aveRun = 0.0;

double Statics::aveDelta = 0.0;
double Statics::aveGamma = 0.0;
int32_t Statics::numOfRlg0_1=0;
double Statics::aveGammaOfBlock=0.0;
double Statics::aveDeltaOfBlock=0.0;

int64_t Statics::M = 1024*1024;
int32_t* Statics::runs_blocks = new int32_t[M];
double Statics::aveRunBlock = 0.0;
//size of everyPart
double Statics::headerSize=0.0;
double Statics::SSize=0.0;
double Statics::plainSize=0.0;
//double Statics::c_dataSize=0.0;
//double Statics::SBrankSize=0.0;
//double Statics::BrankSize=0.0;
double Statics::SBSize=0.0;
double Statics::BSize=0.0;
double Statics::aveRunsOfL=0.0;

void Statics::init(){

	for(int64_t i = 0; i < N; i++){
		runs_num[i] = 0;
	}

	for(int64_t i = 0; i < M; i++){
		runs_blocks[i] = 0;
	}
}