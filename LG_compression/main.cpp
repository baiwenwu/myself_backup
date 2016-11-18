#include<stdlib.h>
#include<string.h>
#include"FM.h"
#include<ctime>
#include<fstream>
#include<iostream>

#include "statics.h"

using namespace std;
#define  times 10000
int main(int argc,char ** argvs)
{
	//-----------kkzone-compress-test------------------
	// if(argc!=2)
	// {
	// 	cout<<"use it like:   ./my_fm file"<<endl;
	// 	exit(0);
	// }
	// time_t t3 = clock();
	// FM fm(argvs[1]);
	// time_t t4 = clock();
	//-----------kkzone-compress-test------------------
	//const char *fileName="./../data/dna.100MB";
	const char *fileName="./../data/english.100MB";
	//const char *fileName="./dna";
	time_t t3 = clock();
	FM fm(fileName);
	time_t t4 = clock();
	cout<<"build time: "<<(t4-t3)/1000000.0<<endl;
	return 0;
}

// int main(int argc, char* argv[])
// {
// 	if(argc < 3){
// 		fprintf(stderr, "Usage: ./my_fm <file> <speedlevel>");
// 		exit(EXIT_FAILURE);
// 	}
// 	fprintf(stderr, "read File %s\n", argv[1]);
	
// 	int speedlevel = atoi(argv[2]);
// 	/*压缩过程*/
// 	FM* fm = new FM(argv[1], speedlevel);
// 	/**/
// 	for(int i = 0; i < 5; i ++){
// 		cout << Statics::coding_styles[i] << " ";
// 	}
// 	cout << endl;
// 	int64_t bitLen = 0;
// 	int64_t sumRun = 0;
// 	//average Delta and Gamma of data
// 	int64_t num_MB=1000*1000;
// 	int64_t sumGamma = 0;
// 	int64_t sumDelta = 0;
// 	int64_t logx = 0;
// 	int64_t runTmp;
// 	//average Delta an Gamma in blocks
// 	sumGamma=0;
// 	sumDelta=0;
// 	logx=0;
// 	sumRun=0;
// 	for(int32_t i = 0; i < Statics::M; i++){
// 		runTmp = Statics::runs_blocks[i];
// 		if(runTmp != 0){
// 			sumRun += runTmp;
// 			logx = Statics::floor_log2(i);
// 			sumGamma += (2*logx + 1)*runTmp;

// 			sumDelta += (logx + 2*Statics::floor_log2(logx + 1) + 1)*runTmp;
// 		}
// 	}
// 	Statics::aveGammaOfBlock = (double)sumGamma / sumRun;
// 	Statics::aveDeltaOfBlock = (double)sumDelta / sumRun;
// 	cout<< " 压缩后Header数据的大小：" << Statics::headerSize << endl;
// 	cout<< " 压缩后数据（不包含附加信息）的总大小: " << Statics::SSize<<endl;
// 	//cout<<"看这儿1:  "<<Statics::fileSize_kkz<<endl;
// 	//cout<<"看这儿1:  "<<Statics::fileSize_kkz/num_MB<<endl;
	
// 	//write into test/<file>.result
// 	char* resFile = new char[1024];
// 	strcpy(resFile, "./test/");
// 	// cout << Statics::getFile(argv[1]);
// 	strcat(resFile, Statics::getFile(argv[1]));
// 	strcat(resFile, ".result");
// 	FILE* out = fopen(resFile, "w");
// 	if(!out){
// 		fprintf(stderr, "Open file %s Error!", resFile);
// 		exit(EXIT_FAILURE);
// 	}

// 	fprintf(stderr, "Write File %s\n", resFile);
	
// 	fprintf(out, "\n ---------------------------压缩后数据统计情况------------------------------\n" );
// 	fprintf(out, " 数据源文件大小：%f MB\n",(double)Statics::fileSize_kkz/num_MB);
// 	fprintf(out, "压缩后Header数据的大小：%f MB\n",(double)Statics::headerSize/num_MB);
// 	fprintf(out, "压缩数据的不包括head信息大小：%f MB\n",Statics::SSize/num_MB);
// 	fprintf(out, "压缩后数据压缩率：%f \n",(Statics::SSize/num_MB+Statics::headerSize/num_MB)/(Statics::fileSize_kkz/num_MB));
// 	fprintf(out, "压缩后数据（ 只包含header信息）压缩率：%f \n",fm->compressRatio());
// 	// fprintf(out, "------------------分块数据的runs的分布计数(run-length-0 + run-length-1)---------------------\n");
// 	// fprintf(out, "\truns的长度\t\truns的个数\n");
// 	// int64_t sumRuns_tmp=0;
// 	// for(int32_t i = 0; i < Statics::M; i++){
// 	// 	if(Statics::runs_blocks[i]){
// 	// 		sumRuns_tmp+=Statics::runs_blocks[i];
// 	// 		fprintf(out, "\t%d\t\t\t\t%d\n",i,sumRuns_tmp);
// 	// 	}
// 	// }

// 	fprintf(out, "\n----------------对应BWT变换后小波树上的bit串进行runs统计-------------------\n");
// 	fprintf(out, "\truns的长度\t\truns的个数\n");
// 	for(int32_t i = 0; i < Statics::N; i++){
// 		if(Statics::runs_num[i]){
// 			fprintf(out, "\t%d\t\t\t\t%d\n", i, Statics::runs_num[i]);
// 		}
// 	}
// 	fclose(out);
// 	fprintf(stderr, "Write File %s end\n\n", resFile);
// 	delete fm;
// 	return 0;
// }