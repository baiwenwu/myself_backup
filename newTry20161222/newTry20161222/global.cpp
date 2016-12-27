#include"global.h"
#include"parameter.h"
#include <fcntl.h>
#include <string>
#include "libdivsufsort\divsufsort.h"
#define BUFF_SIZE 4096
//paramter
//-c :compress
//-d :decompress

//-b :blksize
int blockSiz = 9;//1-9

//-e :tree type
int treeType = 3;//1-3

//-g :nodeCOde
int nodeCode = 1;//1-3

//-v :verbose level
int verbose = 0;//0-2

//-p : number of thread
u32 nthread = 1;//1,2,4

//-k: keep orignal file
int keepOrigFile = 0;//0,1

//-f: force to overwrite outputfile
int overWrite = 0;//0,1

//-h:help manul

//-l:license
int workState = -1;//1,2
// compress  :
//    wzip -c [-b blksize] [-e treetype] [-g node] [-k] [-f] [-p #thread] [-v verbose]filename
// decompress:
//    wzip -d  [-k ] [ -f ] [-v verbose ]filename
// helpInfo  :
//    wzip [-h]
// license   :
//    wzip [-l]
fileInfo_t fileInfo;
//thread[0] is for main thread.
threadInfo_t threadInfos[MAX_THREADS + 1];
char *strcpy1(char *dst, const char *src)
{
	if (!dst&&!src)
	{
		cout << "function: parameters of strcpy is error!" << endl;
	}
	char *ret = dst;
	while ((*dst++ = *src++) != '\0');
	return ret;
}

int threadBlkInit(Stream_t *streamPtr, int index){
	int ret;
	streamPtr->workState = workState ? DECPRESS : COMPRESS;
	streamPtr->blkSiz100k = blockSiz;
	streamPtr->treeShape = shapeMap(treeType);
	streamPtr->nodeCode = nodeCodeTypeMap(nodeCode);
	strcpy1(streamPtr->infileName, fileInfo.orgfileName);

	//sprintf(streamPtr->oufileName, "%s.%d", fileInfo.zipFileName, index);
	sprintf_s(streamPtr->oufileName, "%s.%d", fileInfo.zipFileName, index);


	streamPtr->infd.open(streamPtr->infileName, O_RDONLY);//以只读方式打开文件
	if (streamPtr->infd){
		printf("open error\n");
		exit(0);
	}
	streamPtr->oufd.open(streamPtr->oufileName, O_WRONLY | O_CREAT, 0777);//最后一个参数是指定权限
	if (streamPtr->oufd){
		printf("open error\n");
		exit(0);
	}
	streamPtr->curBlkSeq = 0;
	streamPtr->blkOrigSiz = 0;
	streamPtr->blkAfterSiz = 0;
	streamPtr->verboseLevel = verbose;
	streamPtr->myAlloc = malloc;
	streamPtr->myFree = free;

	streamPtr->inbuff = (uchar*)streamPtr->myAlloc(streamPtr->blkSiz100k\
		* 100000 + sizeof(uchar) + OVERSHOOT
		);//OVERSHOOT=1000 wzip.h define定义
	if (streamPtr->inbuff == NULL){
		return ERR_MEMORY;
	}

	streamPtr->outbuff = (uchar*)streamPtr->myAlloc(streamPtr->blkSiz100k\
		* 100000 * 2
		);
	if (streamPtr->outbuff == NULL){
		return ERR_MEMORY;
	}
	//创建后缀数组,
	streamPtr->suffixArray = (u32*)streamPtr->myAlloc(streamPtr->blkSiz100k\
		* 100000 * sizeof(u32) + sizeof(u32)
		);
	if (streamPtr->suffixArray == NULL){
		return ERR_MEMORY;
	}

	streamPtr->bwt = (uchar *)streamPtr->myAlloc(streamPtr->blkSiz100k\
		* 100000 + sizeof(uchar)
		);
	if (streamPtr->bwt == NULL)
	{
		return ERR_MEMORY;
	}

	memset(streamPtr->charMap, 0, sizeof(streamPtr->charMap));
	memset(streamPtr->charFreq, 0, sizeof(streamPtr->charFreq));
	memset(streamPtr->codeTable, 0, CHAR_SET_SIZE*CODE_MAX_LEN);

	//累计输入数据的上下界值
	streamPtr->totalInHig32 = 0;
	streamPtr->totalInLow32 = 0;

	streamPtr->totalOuHig32 = 0;
	streamPtr->totalOuLow32 = 0;
	//小波数的根结点
	streamPtr->root = NULL;

	//累积cpu时间
	streamPtr->accCpuTime = 0;
	//累积字符bit数
	streamPtr->accBitsPerChar = -1;
	//累积算出的压缩率
	streamPtr->accRatio = -1;
	/*
	lseek函数获取当前完文件的偏移量, very important//linux//log201612261121
	*/
	if (streamPtr->infd.tellg()<0){
		printf("tllg error\n");
		exit(0);
	}
	return 0;
}
int treeCode(Stream_t *streamPtr)
{
	int ret = 0;
	uchar *inbuff = streamPtr->bwt;
	u32 len = streamPtr->blkOrigSiz;
	TreeType shape = streamPtr->treeShape;
	if (!inbuff || !len || !streamPtr)
	{
		return ERR_PARAMETER;
	}
	//switch (shape)
	//{

	//case HUFFMAN:
	//	huffmanTree hufTree;
	//	//hufTree = createHuffTree(inbuff, len, streamPtr);
	//	if (!hufTree)
	//	{
	//		errProcess("createHuffTree", ERR_MEMORY);
	//		return -1;
	//	}
	//	//ret = generateHuffCode(hufTree, streamPtr->codeTable);
	//	if (ret<0)
	//	{
	//		errProcess("generateHuffCode", ret);
	//		//destroyHuffTree(hufTree);
	//		return ret;
	//	}
	//	//destroyHuffTree(hufTree);
	//	return 0;
	//	break;
	//case BALANCE:
	//	balanceTree balTree;
	//	//balTree = createBalanceTree(inbuff, len, streamPtr);
	//	if (!balTree)
	//	{
	//		errProcess("createBalanceTree", ERR_MEMORY);
	//		return -1;
	//	}
	//	//ret = generateBalCode(balTree, streamPtr->codeTable);
	//	if (ret<0)
	//	{
	//		errProcess("generateBalCode", ret);
	//		//destroyBalTree(balTree);
	//		return -1;
	//	}

	//	//destroyBalTree(balTree);
	//	return 0;
	//	break;
	//case HU_TACKER:
	//	hutackerTree hutTree;
	//	//hutTree = createHutackerTree(inbuff, len, streamPtr);
	//	if (!hutTree)
	//	{
	//		errProcess("createHutackerTree", ERR_MEMORY);
	//		return -1;
	//	}

	//	//ret = generateHutackerCode(hutTree, streamPtr->codeTable);
	//	if (ret<0)
	//	{
	//		errProcess("generateHutackerCode", ret);
	//		//destroyHutackerTree(hutTree);
	//		return ret;
	//	}
	//	//destroyHutackerTree(hutTree);
	//	return 0;
	//	break;

	//}


	return 0;

}
int streamBlkCompressCleanUp(Stream_t *streamPtr){
	if (streamPtr == NULL)
	{
		return ERR_PARAMETER;
	}

	streamPtr->infd.close();
	streamPtr->oufd.close();
	if (streamPtr->inbuff)
	{
		streamPtr->myFree(streamPtr->inbuff);
		streamPtr->inbuff = NULL;
	}
	if (streamPtr->outbuff)
	{
		streamPtr->myFree(streamPtr->outbuff);
		streamPtr->outbuff = NULL;
	}


	if (streamPtr->suffixArray)
	{
		streamPtr->myFree(streamPtr->suffixArray);
		streamPtr->suffixArray = NULL;
	}

	if (streamPtr->bwt)
	{
		streamPtr->myFree(streamPtr->bwt);
		streamPtr->bwt = NULL;
	}

	if (streamPtr->root)
	{
		//destroy wavelet tree
		streamPtr->root = NULL;
	}

	return 0;
}

/*    压缩程序入口*/
void *childThread(int i){
	int ret;

	Stream_t stream;
	ret = threadBlkInit(&stream, 0);//log201612261126
	if (ret<0){
		printf("threadBlkInit\n");
		exit(0);
	}

	u32 nread;
	int blkCount = 0;
	// 求SA数组和BWT变换后的L串
	while (blkCount<threadInfos[0].nblocks)
	{
		blkCount++;//important
		nread = stream.blkSiz100k * BLOCKTIMES;
		//printf("每次压缩处理文件的大小:%d\n",nread);
		stream.infd.read((char*)stream.inbuff, nread);
		//ret = read(stream.infd, stream.inbuff, nread);
		if (ret<0){
			printf("read error\n");
			exit(0);
		}
		nread = ret;


		stream.blkOrigSiz = nread + 1;
		stream.inbuff[stream.blkOrigSiz - 1] = '\0';
		if (stream.blkOrigSiz == 1)
		{
			//eof
			break;
		}

		//while computing the suffix array,the lib may use global varbile
		//求得SA数组stream.suffixArray
		ret = divsufsort(stream.inbuff, (int *)stream.suffixArray, stream.blkOrigSiz);
		//ret = blockSort(stream.inbuff,stream.suffixArray,stream.blkOrigSiz,	&(stream.bwtIndex));
		if (ret<0)
		{
			errProcess("blockSort", ret);
			exit(0);
		}
		//获得BWT变换的后的L串即stream.bwt
		int bwtIndex;
		ret = bw_transform(stream.inbuff, stream.bwt, (int *)stream.suffixArray,
			stream.blkOrigSiz, &bwtIndex);
		//ret = getBwtTransform(stream.inbuff, stream.suffixArray,stream.bwt, stream.blkOrigSiz);
		if (ret<0)
		{
			errProcess("getBwtTransform", ret);
			exit(0);
		}
		//获得取字符的编码,根据树形进行不同的编码
		//ret=treeCode(stream.bwt,stream.blkOrigSiz,stream.treeShape,&stream);
		ret = treeCode(&stream);
		if (ret<0)
		{
			errProcess("treeCode", ret);
			exit(0);
		}
		waveletTree root = NULL;
		//waveletTree root = createWaveletTree(stream.bwt,stream.blkOrigSiz,stream.codeTable);

		if (!root)
		{
			errProcess("createWaveletTree", ERR_MEMORY);
			exit(0);
		}

		stream.root = root;//set the element of stream

		//compress bits-vector of wavelet tree
		//ret=compressWaveletTree(stream.root,stream.nodeCode);
		//ret = compressWaveletTree_bww1(&stream);
		if (ret<0)
		{
			errProcess("compressWaveletTree", ret);
			exit(0);
		}
		int zipLen = 0;
		//int zipLen = computeZipSizWaveletTree(stream.root);
		/*if (zipLen<0)
		{
			errProcess("computeZipSizWaveletTree", zipLen);
			exit(0);
		}*/
		if (stream.nodeCode == HBRID)
		{//log201612261440
			uchar block_t = (stream.HBblockSize >> 8) & 0xff;
			stream.oufd.write((char *)&block_t, sizeof(uchar));
		}
		ret = writeBlkCharSetMap(stream.oufd, stream.charMap);
		if (ret<0)
		{
			errProcess("writeBlkCharSetMap", ret);
			exit(0);
		}

		ret = writeBlkCharCodeTable(stream.oufd, stream.codeTable);
		if (ret<0)
		{
			errProcess("writeBlkCharCodeTable", ret);
			exit(0);
		}

		ret = writeBlkBwtIndex(stream.oufd, stream.bwtIndex);
		if (ret<0)
		{
			errProcess("writeBlkBwtIndex", ret);
			exit(0);
		}

		//ret = writeBlkZipNodeWithPreorder(&stream);
		ret = writeBlkZipNodeWithPreorder(stream.oufd, stream.root);
		if (ret<0)
		{
			errProcess("writeBlkZipNodeWithPreorder", ret);
			exit(0);
		}


		stream.totalInLow32 += stream.blkOrigSiz;
		if (stream.totalInLow32<stream.blkOrigSiz)
		{
			//means totalInlow overflow
			stream.totalInHig32++;
		}

		stream.totalOuLow32 += zipLen;
		if (stream.totalOuLow32<zipLen)
		{
			//means totalOuLow overflow
			stream.totalOuHig32++;
		}
		//ret = destroyWaveletTree(stream.root);
		if (ret<0)
		{
			errProcess("destroyWaveletTree", ret);
			exit(0);
		}
		stream.root = NULL;

	}

	ret = streamBlkCompressCleanUp(&stream);
	if (ret<0)
	{
		errProcess("streamBlkCleanUp", ret);
		exit(0);
	}
	//GetLocalTime(&(threadInfos[0].startTime));
	GetLocalTime(&threadInfos[0].endTime);
}
int writeCompressArguments(ofstream &oufd)//log201612261359
{
	if (!oufd)
	{
		return ERR_PARAMETER;
	}

	uchar blkSiz;
	uchar nodeType;

	blkSiz = blockSiz;
	nodeType = nodeCodeTypeMap(nodeCode);

	oufd.write((char *)&blkSiz, sizeof(uchar));//write(oufd, &blkSiz100k, sizeof(uchar));

	oufd.write((char*)&nodeType, sizeof(uchar));//write(oufd, &nodeType, sizeof(uchar));
	return 0;
}
int mainThreadCompressInit(void)
{
	int ret;
	int i;
	ret = filenameMap(fileInfo.orgfileName,
		fileInfo.zipFileName,
		COMPRESS
		);
	if (ret<0)
	{
		return ERR_FILE_NAME;
	}

	//state函数是通过文件名filename获取文件信息，并保存在对应的结构体stat中
	if (stat(fileInfo.orgfileName, &fileInfo.fileStat)<0){
		printf("stat error\n");
		return ERR_FILE_NAME;
	}
	fileInfo.orgFileSize = fileInfo.fileStat.st_size;

	if (!keepOrigFile && fileInfo.fileStat.st_nlink>1){
		cout << fileInfo.orgfileName<< " has more than one hard-link,may be you need -k option!" << endl;
		exit(0);
	}
	//log201612261222
	fileInfo.totalBlks = fileInfo.orgFileSize / (BLOCKTIMES * blockSiz) + \
		(fileInfo.orgFileSize % (BLOCKTIMES * blockSiz) ? 1 : 0)
		;

	printf("totalBlks :%d\n", fileInfo.totalBlks);
	if (fileInfo.totalBlks<nthread){//log201612261304
		nthread = fileInfo.totalBlks;
	}

	fileInfo.accCompress = 0;
	fileInfo.bitsPerChar = 0;
	fileInfo.ratio = 0;



	int blksPerThread = fileInfo.totalBlks / nthread;
	printf("blksPerThread:%d\n", blksPerThread);

	for (i = 1; i <= nthread; i++){
		memset(&threadInfos[i], 0, sizeof(threadInfo_t));
		threadInfos[i].nblocks = blksPerThread;
	}

	int ncurBlks = blksPerThread*nthread;
	printf("ncurBlks=%d\n", ncurBlks);
	while (ncurBlks<fileInfo.totalBlks){
		threadInfos[(ncurBlks%nthread) + 1].nblocks++;
		ncurBlks++;
	}
	//set the file offset
	off_t pos = 0;
	for (i = 1; i <= nthread; i++){//log201612261222
		threadInfos[i].fileOffset = pos;
		pos += threadInfos[i].nblocks*blockSiz * BLOCKTIMES;
	}

#if 1
	//for show file offset
	for (i = 1; i <= nthread; i++){
		printf("thread %i: %u\n", i, threadInfos[i].fileOffset);
	}
#endif

	//check weather outfile already exist
	struct stat temp;
	if (stat(fileInfo.zipFileName, &temp) == 0 &&
		!overWrite
		){
		printf("%s has already exist,you may need -f option to overwrite\n",
			fileInfo.zipFileName
			);
		exit(0);
	}
	//打开文件其实质是创建压缩文件
	ofstream oufd;
	oufd.open(fileInfo.zipFileName, O_WRONLY | O_CREAT, 0777 | ios::trunc);//log201612261313
	if (!oufd.is_open())
	{
		printf("open error\n");
		exit(0);
	}
	//important////log201612261313
	ret = writeFileHeader(oufd);
	if (ret<0){
		errProcess("writeFileHeader", ret);
		exit(0);
	}

	ret = writeCompressArguments(oufd);//
	if (ret<0){
		errProcess("writeCompressArguments", ret);
		exit(0);
	}
	oufd.close();
	return 0;
}
void compressMainThread(void){
	int ret;

	char fileName[MAX_FILE_LEN];

	GetLocalTime(&(threadInfos[0].startTime));
	ret = mainThreadCompressInit();
	if (ret<0){
		printf("mainThreadCompress error\n");
		exit(0);
	}

	for (int i = 1; i <= nthread; i++)
	{
		childThread(i);
	}
	ofstream zipfd;
	zipfd.open(fileInfo.zipFileName, O_WRONLY | O_APPEND);
	if (!zipfd.is_open()){
		printf("open error\n");
		exit(0);
	}

	int tempfd;

	uchar buff[BUFF_SIZE];//BUFF_SIZE=4096
	u32 nread;

	//writeZipInfo
	uchar ch_thread = nthread & 0xff;
	zipfd.write((char*)&ch_thread, sizeof(uchar));//log201612261558
	struct stat statBuff;
	//off_t offset = lseek(zipfd, 0, SEEK_CUR) + 	nthread*(sizeof(off_t) + sizeof(u32));
	off_t offset = 0;
	//printf("start pos %lld\n", lseek(zipfd, 0, SEEK_CUR));
	char tempFile[256];
	for (int i = 1; i <= nthread; i++)
	{
		zipfd.write((char*)&offset, sizeof(off_t));//log201612261558
		zipfd.write((char*)&threadInfos[i].nblocks, sizeof(int));
		printf("offset=%lld,blocks=%d\n", offset, threadInfos[i].nblocks);
		sprintf_s(tempFile, "%s.%d", fileInfo.zipFileName, i);//sprintf
		if (stat(tempFile, &statBuff)<0){
			printf("stat error\n");
			exit(0);
		}
		//printf("file %s:size %lld\n",tempFile,statBuff.st_size);
		offset += statBuff.st_size;//statBuff.st_size临时文件大小
	}

	printf("Merge files Start\n");
	for (int i = 1; i <= nthread; i++){
		sprintf_s(fileName, "%s.%d", fileInfo.zipFileName, i);//将格式化数据写入buff(fileName)

		//open file
		ifstream tempfd; 
		tempfd.open(fileName, O_RDONLY);
		if (!tempfd.is_open()){
			printf("open error\n");
			exit(0);
		}
		while (1){//将一个文件读完循环停止
			nread = sizeof(buff);
			tempfd.read((char*)buff, nread);//ret = read(tempfd, buff, nread);
			ret = tempfd.tellg();
			if (ret<0)
			{
				errProcess("read error", ret);
				exit(0);
			}
			else if (ret == 0)
			{
				//eof
				break;
			}
			zipfd.write((char*)buff, ret);
			//write(zipfd, buff, ret);
		}

		//eof
		tempfd.close();
		//remove tempfile
		remove(fileName);
	}


	writeFileEnd(zipfd);
	GetLocalTime(&(threadInfos[0].endTime));
	
	struct stat tempBuff;
	if (verbose == 1){
		printf("compress done.\n");
	}
	else if (verbose == 2){
		if (stat(fileInfo.zipFileName, &tempBuff) != 0){
			printf("stat error\n");
			exit(0);
		}
		printf("compress done.\n");
		printf("\tratio:%.3f%%,\tbits/char:%.3f\n",
			tempBuff.st_size*100.0 / fileInfo.orgFileSize,
			tempBuff.st_size*8.0 / fileInfo.orgFileSize
			);
		//log201612261718
	}

	//remove the orignal file
	if (!keepOrigFile){
		remove(fileInfo.orgfileName);
	}
	zipfd.close();
}



