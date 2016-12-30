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
int getParameters()
{
	blockSiz = 9;//b
	if (blockSiz <= 0 ||
		blockSiz>9
		){
		printf("blockSize should within [1-9]\n");
		return ERR_PARAMETER;
	}
	treeType = 1;//e
	if (treeType <= 0 ||
		treeType > 3
		){
		printf("treetype should within [1-3]\n");
		return ERR_PARAMETER;
	}
	nodeCode = 1;//g
	if (nodeCode <= 0 ||
		nodeCode>2
		){
		printf("nodeCode should within [1-2]\n");
		return ERR_PARAMETER;
	}
	verbose = 1;//v
	if (verbose<0 ||
		verbose>2
		){

		printf("verbose should witin [0-2]\n");
		return ERR_PARAMETER;
	}
	keepOrigFile = 1;//k
	overWrite = 1;//f
	//d
	if (workState == -1){
		workState = 0;
	}
	else if (workState == 0){
		printf("more than two option -c \n");
		return ERR_PARAMETER;
	}
	else{
		printf("can't have both option -d and option -c \n");
		return ERR_PARAMETER;
	}
	nthread = 1;//p
	if (nthread <= 0 ||
		nthread>4
		){
		printf("nthread should with [1-4]\n");
		return ERR_PARAMETER;
	}
		
	strcpy_s(fileInfo.orgfileName, FILE_NAME_LEN,"management-server.log");
	return 0;
}

int threadBlkInit(Str_rt *str_r, int index){
	int ret;
	Stream_t *streamPtr = &str_r->str_s;
	streamPtr->workState = workState ? DECPRESS : COMPRESS;
	streamPtr->blkSiz100k = blockSiz;
	streamPtr->treeShape = shapeMap(treeType);
	streamPtr->nodeCode = nodeCodeTypeMap(nodeCode);
	strcpy1(streamPtr->infileName, fileInfo.orgfileName);

	//sprintf(streamPtr->oufileName, "%s.%d", fileInfo.zipFileName, index);
	sprintf_s(streamPtr->oufileName, "%s.%d", fileInfo.zipFileName, index);

	streamPtr->infd.open(streamPtr->infileName);//以只读方式打开文件  O_RDONLY
	if (!streamPtr->infd.is_open()){
		printf("open error\n");
		exit(0);
	}
	streamPtr->oufd.open(streamPtr->oufileName, ios::out);//最后一个参数是指定权限
	if (!streamPtr->oufd.is_open()){
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
		* BLOCKTIMES + sizeof(uchar) + OVERSHOOT
		);//OVERSHOOT=1000 wzip.h define定义
	if (streamPtr->inbuff == NULL){
		return ERR_MEMORY;
	}

	streamPtr->outbuff = (uchar*)streamPtr->myAlloc(streamPtr->blkSiz100k\
		* BLOCKTIMES * 2
		);
	if (streamPtr->outbuff == NULL){
		return ERR_MEMORY;
	}
	//创建后缀数组,
	streamPtr->suffixArray = (u32*)streamPtr->myAlloc(streamPtr->blkSiz100k\
		* BLOCKTIMES * sizeof(u32) + sizeof(u32)
		);
	if (streamPtr->suffixArray == NULL){
		return ERR_MEMORY;
	}

	streamPtr->bwt = (uchar *)streamPtr->myAlloc(streamPtr->blkSiz100k\
		* BLOCKTIMES + sizeof(uchar)
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
	str_r->root = NULL;

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
int treeCode(Str_rt *str_r)
{
	Stream_t *streamPtr = &str_r->str_s;
	int ret = 0;
	uchar *inbuff = streamPtr->bwt;
	u32 len = streamPtr->blkOrigSiz;
	TreeType shape = streamPtr->treeShape;
	shape = HU_TACKER;
	if (!inbuff || !len || !streamPtr)
	{
		return ERR_PARAMETER;
	}
	switch (shape)
	{

	case HUFFMAN:
		huffmanTree hufTree;
		hufTree = createHuffTree(inbuff, len, streamPtr);
		if (!hufTree)
		{
			errProcess("createHuffTree", ERR_MEMORY);
			return -1;
		}
		ret = generateHuffCode(hufTree, streamPtr->codeTable);
		if (ret<0)
		{
			errProcess("generateHuffCode", ret);
			destroyHuffTree(hufTree);
			return ret;
		}
		destroyHuffTree(hufTree);
		return 0;
		break;
	case BALANCE:
		balanceTree balTree;
		balTree = createBalanceTree(inbuff, len, streamPtr);
		if (!balTree)
		{
			errProcess("createBalanceTree", ERR_MEMORY);
			return -1;
		}
		ret = generateBalCode(balTree, streamPtr->codeTable);
		if (ret<0)
		{
			errProcess("generateBalCode", ret);
			destroyBalTree(balTree);
			return -1;
		}

		destroyBalTree(balTree);
		return 0;
		break;
	case HU_TACKER:
		hutackerTree hutTree;
		hutTree = createHutackerTree(inbuff, len, streamPtr);
		if (!hutTree)
		{
			errProcess("createHutackerTree", ERR_MEMORY);
			return -1;
		}

		ret = generateHutackerCode(hutTree, streamPtr->codeTable);
		if (ret<0)
		{
			errProcess("generateHutackerCode", ret);
			destroyHutackerTree(hutTree);
			return ret;
		}
		destroyHutackerTree(hutTree);
		return 0;
		break;

	}
	return 0;

}
int streamBlkCompressCleanUp(Str_rt *str_r){
	Stream_t *streamPtr = &str_r->str_s;
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

	if (str_r->root)
	{
		//destroy wavelet tree
		str_r->root = NULL;
	}

	return 0;
}

/*    压缩程序入口*/
void *childThread(int i){
	int ret = 0;
	Str_rt str_r;
	Stream_t &stream = str_r.str_s; //log201612282342
	ret = threadBlkInit(&str_r, 1);//log201612261126
	if (ret<0){
		cout <<"threadBlkInit\n" << endl;
		exit(0);
	}

	u32 nread;
	int blkCount = 0;
	// 求SA数组和BWT变换后的L串
	while (blkCount < threadInfos[1].nblocks)
	{
		blkCount++;//important
		nread = stream.blkSiz100k * BLOCKTIMES;
		//printf("每次压缩处理文件的大小:%d\n",nread);
		stream.infd.read((char*)stream.inbuff, nread);
		nread=stream.infd.gcount();
		//ret = read(stream.infd, stream.inbuff, nread);
		if (ret < 0){
			cout << "read error\n" << endl;
			exit(0);
		}
		//nread = ret;

	
		stream.blkOrigSiz = nread + 1;
		stream.inbuff[stream.blkOrigSiz - 1] = '\0';
		if (stream.blkOrigSiz == 0)
		{
			//eof
			break;
		}
#if 1
		u32 readCount = stream.infd.gcount();
		cout << "-----" << blkCount << "-----" << endl;
		cout << "\tgcount=\t" << readCount << endl;
		cout << "\tnread=\t" << nread << endl;
		cout << "\tlen=\t" << strlen((char*)stream.inbuff) << endl;
#endif

		//while computing the suffix array,the lib may use global varbile
		ret = divsufsort(stream.inbuff, (int *)stream.suffixArray, stream.blkOrigSiz);
		if (ret < 0)
		{
			errProcess("blockSort", ret);
			exit(0);
		}
		int bwtIndex;
		ret = bw_transform(stream.inbuff, stream.bwt, (int *)stream.suffixArray,
			stream.blkOrigSiz, &bwtIndex);
		//ret = getBwtTransform(stream.inbuff, stream.suffixArray,stream.bwt, stream.blkOrigSiz);
		if (ret < 0)
		{
			errProcess("getBwtTransform", ret);
			exit(0);
		}

		ret = treeCode(&str_r);
		if (ret<0)
		{
			errProcess("treeCode", ret);
			exit(0);
		}

		str_r.root = createWaveletTree(stream.bwt, stream.blkOrigSiz, stream.codeTable);
		if (!str_r.root)
		{
			errProcess("createWaveletTree", ERR_MEMORY);
			exit(0);
		}

		//compress bits-vector of wavelet tree
		ret = compressWaveletTree(str_r.root,stream);
		if (ret<0)
		{
			errProcess("compressWaveletTree", ret);
			exit(0);
		}

		int zipLen = computeZipSizWaveletTree(str_r.root);
		if (zipLen<0)
		{
			errProcess("computeZipSizWaveletTree", zipLen);
			exit(0);
		}

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
		ret = writeBlkZipNodeWithPreorder(stream.oufd, str_r.root);
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
		str_r.root = NULL;
	}
	ret = streamBlkCompressCleanUp(&str_r);
	if (ret<0)
	{
		errProcess("streamBlkCleanUp", ret);
		exit(0);
	}
	GetLocalTime(&threadInfos[0].endTime);
     return NULL;
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
	cout << "文件大小：" << fileInfo.orgFileSize << endl;
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
	//oufd.open(fileInfo.zipFileName, O_WRONLY | O_CREAT, 0777 | ios::trunc);//log201612261313
	oufd.open(fileInfo.zipFileName, ios::out | ios::trunc);
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
	//--------baiwenwu-c-----
	int count_t = zipfd.tellp();
	off_t offset = (off_t)zipfd.tellp() + nthread*(sizeof(off_t) + sizeof(u32));
	//printf("start pos %lld\n", lseek(zipfd, 0, SEEK_CUR));
	char tempFile[256];
	for (int i = 1; i <= nthread; i++)
	{
		zipfd.write((char*)&offset, sizeof(off_t));//log201612261558
		zipfd.write((char*)&threadInfos[i].nblocks, sizeof(int));
		cout << "offset=" << offset << "\tblocks=" << threadInfos[i].nblocks << endl;
		//printf("offset=%lld,blocks=%d\n", offset, threadInfos[i].nblocks);
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
		tempfd.open(fileName, ios::in|ios::binary);
		if (!tempfd.is_open()){
			printf("open error\n");
			exit(0);
		}
#if 0
		ret = tempfd.tellg();
		while (1)
		{
			char ch_t[101];
			tempfd.read(ch_t, 100);
			ch_t[100] = '\0';
			cout << ch_t << endl;
			cout<< "  tellg=" << tempfd.tellg() << "  count=" << tempfd.gcount() << endl;
		}
#endif
		while (1){//将一个文件读完循环停止
			nread = BUFF_SIZE;
			tempfd.read((char*)buff, nread);//ret = read(tempfd, buff, nread);
			ret = tempfd.gcount();
			//ret = tempfd.tellg();
			if (ret==0)
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

void decompressChildProcess(int i, Str_rt *str_rPtr)
{
	if (!str_rPtr)
	{
		printf("parameter error\n");
		exit(0);
	}
	Stream_t *streamPtr = &str_rPtr->str_s;
	streamPtr->infd.close();
	streamPtr->oufd.close();

	char outFileName[256];
	sprintf_s(outFileName, "%s.%d", streamPtr->oufileName, i);
	sprintf_s(streamPtr->oufileName, outFileName);

	streamPtr->infd.open(streamPtr->infileName,ios::in|ios::binary);
	if (!streamPtr->infd.is_open()){
		printf("open error\n");
		exit(0);
	}

	streamPtr->oufd.open(streamPtr->oufileName,ios::out);//(streamPtr->oufileName, O_CREAT | O_WRONLY, 0777);
	if (streamPtr->oufd.is_open()){
		printf("open error\n");
	}
	
	if (streamPtr->infd.tellg()<0){
		printf("lseek error\n");
		exit(0);
	}

	int count = 0;
	int ret;
	while (count<threadInfos[i].nblocks)
	{
		count++;
		ret = paraseBlkCharSetMap(streamPtr);
		if (ret<0)
		{
			//error
			errProcess("paraseBlkCharSetMap", ret);
			streamBlkCompressCleanUp(str_rPtr);
			exit(0);
		}

		ret = paraseBlkCharCodeTable(streamPtr);
		if (ret<0)
		{
			errProcess("praseBlkCharCodeTable", ret);
			exit(0);
		}

		ret = paraseBlkBwtIndex(streamPtr);
		if (ret<0)
		{
			errProcess("paraseBlkBwtIndex", ret);
			exit(0);
		}

		str_rPtr->root = genWavtreeWithCodeTable(streamPtr->codeTable);
		if (!str_rPtr->root)
		{
			errProcess("genWavtreeWithCodeTable", ERR_MEMORY);
			exit(0);
		}

		ret = paraseBlkZipNodeWithPreorder(str_rPtr->root,streamPtr);
		if (ret<0)
		{
			errProcess("paraseBlkZipNodeWithPreorder", ret);
			exit(0);
		}

		ret = genBwtWithWaveletTree(str_rPtr->root, streamPtr);
		if (ret<0)
		{
			errProcess("generateBwtWithWaveletTree", ret);
			exit(0);
		}

		ret = genOrigBlkWithBwt(streamPtr->bwt,
			streamPtr->blkOrigSiz,
			streamPtr->bwtIndex,
			streamPtr->inbuff
			);
		if (ret<0)
		{
			errProcess("genOrigBlkWithBwt", ret);
			exit(0);
		}

		ret = streamWriteOrigBlk(streamPtr);
		if (ret<0)
		{
			errProcess("streamWriteOrigBlk", ret);
			exit(0);
		}

		//very important
		destroyWaveletTree(str_rPtr->root);
		str_rPtr->root = NULL;

	}
	if (str_rPtr->root)
	{
		destroyWaveletTree(str_rPtr->root);
		str_rPtr->root = NULL;
	}
	streamBlkDecompressCleanUp(streamPtr);

}


void decompressMainThread(void){
	int ret;
	Str_rt Str_r;
	Stream_t &stream = Str_r.str_s;
	stream.workState = DECPRESS;

	strcpy_s(stream.infileName, FILE_NAME_LEN, fileInfo.orgfileName);
	GetLocalTime(&threadInfos[0].startTime);

	Str_r.root = NULL;
	ret = 0;
	ret = streamBlkDecompressInit(&stream);
	if (ret<0)
	{
		errProcess("streamBlkDecompressInit", ret);
		exit(0);
	}

	off_t pos;

	//
	stream.infd.read((char*)&nthread, sizeof(int));//read(stream.infd, &nthread, sizeof(int));
	int blocks;
	off_t offset;
	int i;
	for (i = 1; i <= nthread; i++){
		stream.infd.read((char*)&threadInfos[i].fileOffset, sizeof(off_t));
		stream.infd.read((char*)&threadInfos[i].nblocks, sizeof(int));
		//printf ("offset=%lld,blocks=%d\n",
		//     threadInfos[i].fileOffset,threadInfos[i].nblocks);
	}

	//create child process
	for (i = 1; i <= nthread; i++)
	{
		//child process
		decompressChildProcess(i, &Str_r);
	}
	streamBlkDecompressCleanUp(&stream);
	//wait for child process
#if 0;
	if (signal(SIGINT, sig_int_decompress) == SIG_ERR){
		printf("signal error\n");
		exit(0);
	}
	for (i = 1; i <= nthread; i++){
		if (waitpid(threadInfos[i].threadId, NULL, 0)<0){
			printf("waitpid error\n");
			exit(0);
		}
	}
#endif

	//merge for files
	char buff[BUFF_SIZE];
	char tempfile[256];
	ifstream  tempfd;

	stream.oufd.open(stream.oufileName, ios::out|ios::trunc);// O_WRONLY | O_CREAT, 0666);
	if (!stream.oufd.is_open()){
		printf("open error\n");
		exit(0);
	}

	for (i = 1; i <= nthread; i++)
	{
		sprintf_s(tempfile, "%s.%d", stream.oufileName, i);
		tempfd.open(tempfile, ios::in);
		if (!tempfd.is_open()){
			printf("open error\n");
			exit(0);
		}
		while (1){
			tempfd.read((char*)buff, sizeof(buff));
			ret = tempfd.gcount();
			if (ret<0){
				printf("read error\n");
				exit(0);
			}
			else if (ret == 0){
				break;
			}

			stream.oufd.write((char*)buff, ret);
		}

		//need to remove temp file
		remove(tempfile);
	}


	if (!keepOrigFile){
		remove(stream.infileName);
	}
	GetLocalTime(&threadInfos[0].endTime);
	

	if (verbose == 1){
		printf("decompress done.\n");
	}
	else if (verbose == 2){
		printf("decompress done.\n");
#if 0
		struct timeval tmpTime;
		tmpTime.tv_sec = threadInfos[0].endTime.tv_sec - \
			threadInfos[0].startTime.tv_sec;
		tmpTime.tv_usec = threadInfos[0].endTime.tv_usec - \
			threadInfos[0].startTime.tv_usec;
		if (tmpTime.tv_usec<0){
			tmpTime.tv_sec--;
			tmpTime.tv_usec += 1000000;
		}

		printf("real time of mainThread:%.3f(s)\n",
			tmpTime.tv_sec + tmpTime.tv_usec / 1000000.0
			);
#endif
	}
}



