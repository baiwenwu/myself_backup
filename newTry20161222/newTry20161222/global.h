#ifndef GLOBAL_H
#define GLOBAL_H
#include <windows.h> //#include <time.h>
#include <sys/stat.h>  
#include"wzip.h"
#include"wavelet.h"
#include"fileProcess.h"
#include"compress.h"
#include"errProcess.h"
#define _FILE_OFFSET_BITS 64
#define MAX_THREADS 1
#define MAX_FILE_LEN 1024
#define BLOCKTIMES 10485760	//log201612261222

typedef struct Stream_t{
	//input and output filenames
	char infileName[FILE_NAME_LEN];
	char oufileName[FILE_NAME_LEN];

	//in and out file pointer
	FILE *infile;
	FILE *oufile;

	//in and out file descriptor
	ifstream infd;
	ofstream oufd;
	//int infd;
	//int oufd;


	//file size
	long int fileSize;
	// current block sequence
	u32 curBlkSeq;

	// current block size
	u32 blkSiz100k;

	// for compress mode
	u32 blkOrigSiz;
	uchar *inbuff;

	// for decompress mode
	u32 blkAfterSiz;
	uchar *outbuff;
	//current work state
	Mode workState;

	//for compress mode
	TreeType treeShape;

	//the code pattern of the tree nodes
	NodeCodeType nodeCode;

	//-----baiwenwu-c--
	u32 GHBblockSize;//块大小的确认
	//statics info output level
	char verboseLevel;

	//suffix array for bwt transformation
	//for compress mode
	u32* suffixArray;

	//for both compress and decompress mode
	uchar* bwt;
	u32 bwtIndex;
	//base block size just for hybride coding 
	u32 HBblockSize;
	u32 HBlevel;
	u32 speedlevel;

	//for huffman , hu-tacker ,balance ,compute code tabel
	u32 setSize;
	bool charMap[CHAR_SET_SIZE];
	u32	 charFreq[CHAR_SET_SIZE];
	char codeTable[CHAR_SET_SIZE][CODE_MAX_LEN];

	//accumulate input data size
	u32 totalInHig32;
	u32 totalInLow32;

	//accumulate output data size
	u32 totalOuHig32;
	u32 totalOuLow32;

	//wavelet tree
	waveletTree root;



	//current block start time
	time_t	blkStartTime;
	time_t  blkEndTime;
	//current block compress/decompress ratio
	float curRatio;


	//accumulate cup time
	time_t accCpuTime;
	//accumulate bits per char
	float accBitsPerChar;
	//accumulate ratio
	float accRatio;
	void *(*myAlloc)(size_t);
	void(*myFree)(void *);

}Stream_t;

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

int getParameters(int argc, char *argv[]);

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
extern int keepOrigFile;
extern int overWrite; 
#endif