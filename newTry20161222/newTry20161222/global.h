#ifndef GLOBAL_H
#define GLOBAL_H
#include <windows.h> //#include <time.h>
#include <sys/stat.h>  
#include"wzip.h"
#include"errProcess.h"
#include"fileProcess.h"
#include"wavelet.h"
#include"huffman.h"
#include"balance.h"
#include"hutacker.h"
#include"compress.h"
#include"decompress.h"


#define _FILE_OFFSET_BITS 64


typedef struct Str_rt{//stream_root
	Stream_t str_s;
	waveletTree root;
}Str_root;
typedef struct fileInfo_t{
	char orgfileName[MAX_FILE_LEN];
	char zipFileName[MAX_FILE_LEN];

	struct stat fileStat;//windows中可以用这个结构struct _WIN32_FIND_DATA fileStat;代替，但是没有找到其定义的头文件（log20161223）
	long int orgFileSize;
	long int zipFileSize;

	unsigned int totalBlks;//记录整个文件被分成了多少块

	unsigned int accCompress;

	double bitsPerChar;
	double ratio;
}fileInfo_t;

typedef struct threadInfo_t{
	int threadId;//pid_t threadId;
	int nblocks;
	off_t fileOffset;
	SYSTEMTIME startTime;//time_t  startTime;//struct timeval startTime;
	SYSTEMTIME endTime;//time_t  endTime; //struct timeval endTime;
	float threadRatio;
}threadInfo_t;
int getParameters();
//int getParameters(int argc, char *argv[]);

int mainThreadCompressInit(void);
void compressMainThread(void);
void decompressMainThread(void);

void showGlobalValue(void);
void printfHelp(void);
void printfManul(void);
void printLicense(void);


extern fileInfo_t fileInfo;

//thread[0] is for main thread.
extern int workState;
#endif