 #define _FILE_OFFSET_BITS 64
#include "global.h"
#include "wzip.h"
#include "fileProcess.h"
#include "parameter.h"
#include "compress.h"
#include "errProcess.h"
#include "blocksort.h"
#include "huffman.h"
#include "hutacker.h"
#include "balance.h"
#include "wavelet.h"
#include "rleElisCode.h"
#include "decompress.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFF_SIZE 4096
//paramter
//-c :compress
//-d :decompress

//-b :blksize
int blockSiz=9;//1-9

//-e :tree type
int treeType=3;//1-3

//-g :nodeCOde
int nodeCode=3;//1-3

//-v :verbose level
int verbose=2;//0-2

//-p : number of thread
int nthread=4;//1,2,4

//-k: keep orignal file
int keepOrigFile=0;//0,1

//-f: force to overwrite outputfile
int overWrite=0;//0,1

//-h:help manul

//-l:license

int workState=-1;//1,2



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
threadInfo_t threadInfos[MAX_THREADS+1];

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

//bybird blockSize
u32 HBblockSize=0;

void sig_int_compress(int signo){
    char fileName[MAX_FILE_LEN];
    printf("wzip interrupted unexpectedly,do some cleanup\n");
    int i;
    for(i=1;i<=nthread;i++){
        sprintf (fileName,"%s.wz.%i",fileInfo.orgfileName,i);
        printf("remove %s\n",fileName);
        unlink (fileName);
    }

    sprintf (fileName,"%s.wz",fileInfo.orgfileName);
    unlink(fileName);
    exit(0);
}

void sig_int_decompress(int signo){
    printf("wzip interrupted unexpectedly,do some cleanup\n");
    char fileName[MAX_FILE_LEN];
    filenameMap (fileInfo.orgfileName,fileName,DECPRESS);
    unlink(fileName);
    printf("remove %s\n",fileName);
    int i;

    char tempfile[MAX_FILE_LEN];
    for(i=1;i<=nthread;i++){
        sprintf (tempfile,"%s.%d",fileName,i);
        unlink (tempfile);
    }
    exit(0);
}


void printfHelp(void){
    printf("compress:\n"
           "\twzip -c [-b blksize] [-e treetype] [-g node] [-k] [-f] "
           "[-p #thread] [-v verbose] filename\n"
           "decompress:\n"
           "\twzip -d  [-k ] [ -f ] [-v ] [-v verbose ]filename\n"
           "helpInfo:\n"
           "\twzip [-h]\n"
           "license:\n"
           "\twzip [-L]\n"
           );
}

void printfManul(void){
    system ("cat manul.txt|more");
    exit(0);
}

void printLicense(void){
    system ("cat README|more");
    exit(0);
}


void showGlobalValue(void){
    printf ("Global value:\n");
    printf ("\tblockSize=%d\n",blockSiz);
    printf ("\ttreeType=%d\n",treeType);
    printf("\tcodeType=%d\n",nodeCode);
    printf("\tverbose=%d\n",verbose);
    printf("\tnthread=%d\n",nthread);
    printf ("\tkeepOrigFile=%d\n",keepOrigFile);
    printf("\toverWrite=%d\n",overWrite);
    printf ("\tworkStat=%d\n",workState);
    printf ("fileName=%s\n",fileInfo.orgfileName);

}

int getParameters (int argc, char *argv[])
{
    int c;
    while((c=getopt (argc,argv,"b:e:g:v:p:kfhldc"))
                !=-1
          )
    {

        switch(c){
        case 'b':
            //printf("option -b\n");
            blockSiz=atoi(optarg);
            if(blockSiz<=0 ||
                    blockSiz>9
              ){
                printf ("blockSize should within [1-9]\n");
                return ERR_PARAMETER;
            }
            break;
        case 'e':
            treeType=atoi(optarg);
            if(treeType<=0||
                    treeType>3
               ){
                printf ("treetype should within [1-3]\n");
                return ERR_PARAMETER;
            }
            break;
        case 'g':
            nodeCode=atoi(optarg);
            if(nodeCode<=0||
                     nodeCode>3
               ){
                printf ("nodeCode should within [1-3]\n");
                return ERR_PARAMETER;
            }
            break;
        case 'v':
            verbose=atoi(optarg);
            if(verbose<0||
                    verbose>2
               ){

                printf ("verbose should witin [0-2]\n");
                return ERR_PARAMETER;
            }
            break;
        case 'k':
            keepOrigFile=1;
            break;
        case 'f':
            overWrite=1;
            break;
        case 'c':
            if(workState==-1){
                workState=0;
            }else if(workState==0){
                printf("more than two option -c \n");
                return ERR_PARAMETER;
            }else{
                printf("can't have both option -d and option -c \n");
                return ERR_PARAMETER;
            }

            break;
        case 'd':
            if(workState==-1){
                workState=1;
            }else if(workState==0){
                printf("can't have both option -d and option -c \n");
                return ERR_PARAMETER;
            }else{
                printf ("more than two option -d\n");
                return ERR_PARAMETER;
            }
            break;
        case 'p':
            nthread=atoi(optarg);
            if(nthread<=0 ||
                    nthread>4
              ){
                printf ("nthread should with [1-4]\n");
                return ERR_PARAMETER;
            }
            break;
        case 'h':
            printfHelp ();
            //exit in advance
            exit(0);
            break;

        case 'l':
            printLicense ();
            //exit in advance
            exit(0);
            break;

        }

    }

    if(optind!=argc-1){
        return ERR_PARAMETER;
    }

    strcpy (fileInfo.orgfileName,argv[optind]);
    return 0;
}


int writeCompressArguments(int oufd)
{
    if (oufd<0)
    {
        return ERR_PARAMETER;
    }

    uchar blkSiz100k;
    uchar nodeType;

    blkSiz100k=blockSiz;
    nodeType  =nodeCodeTypeMap (nodeCode);

    //fwrite(&blkSiz100k,sizeof(uchar),1,zipFile);
    write(oufd,&blkSiz100k,sizeof(uchar));

    //fwrite(&nodeType,sizeof(uchar),1,zipFile);
    write(oufd,&nodeType,sizeof(uchar));
    return 0;
}


//init fileInfo and write zipFileHeader
int mainThreadCompressInit(void)
{
    int ret;
    int i;
    ret=filenameMap(fileInfo.orgfileName,
                        fileInfo.zipFileName,
                            COMPRESS
                );
    if (ret<0)
    {
        return ERR_FILE_NAME;
    }

    if(stat(fileInfo.orgfileName,&fileInfo.fileStat)<0){
        printf("stat error\n");
        return ERR_FILE_NAME;
    }

    fileInfo.orgFileSize=fileInfo.fileStat.st_size;

    if(!keepOrigFile && fileInfo.fileStat.st_nlink>1){
        printf("%s has more than one hard-link,may be you need -k option!",
                fileInfo.orgfileName
               );
        exit(0);
    }

    fileInfo.totalBlks=fileInfo.orgFileSize/(100000*blockSiz)+\
                            (fileInfo.orgFileSize%(100000*blockSiz)?1:0)
                        ;

   // printf("totalBlks :%d\n",fileInfo.totalBlks);
    if(fileInfo.totalBlks<nthread){
        nthread=fileInfo.totalBlks;
    }

    fileInfo.accCompress=0;
    fileInfo.bitsPerChar=0;
    fileInfo.ratio=0;



    int blksPerThread=fileInfo.totalBlks/nthread;
    //printf ("blksPerThread:%d\n",blksPerThread);

    for(i=1;i<=nthread;i++){
        memset (&threadInfos[i],0,sizeof(threadInfo_t));
        threadInfos[i].nblocks=blksPerThread;
    }

    int ncurBlks=blksPerThread*nthread;
    //printf ("ncurBlks=%d\n",ncurBlks);
    while(ncurBlks<fileInfo.totalBlks){
        threadInfos[(ncurBlks%nthread)+1].nblocks++;
        ncurBlks++;
    }
    //set the file offset
    off_t pos=0;
    for(i=1;i<=nthread;i++){
        threadInfos[i].fileOffset=pos;
        pos+=threadInfos[i].nblocks*blockSiz*100000;
    }

#if 0
    //for show file offset
    for(i=1;i<=nthread;i++){
        printf("thread %i: %u\n",i,threadInfos[i].fileOffset);
    }
#endif

    //check weather outfile already exist
    struct stat temp;
    if(stat(fileInfo.zipFileName,&temp)==0 &&
                !overWrite
      ){
        printf("%s has already exist,you may need -f option to overwrite\n",
                fileInfo.zipFileName
               );
        exit(0);
    }


    int oufd;
    oufd=open(fileInfo.zipFileName,O_WRONLY|O_CREAT,0777);
    if(oufd<0)
    {
        printf("open error\n");
        exit(0);
    }
    //important
    if(ftruncate (oufd,0L)<0)
    {
        printf ("ftruncate error\n");
        exit(0);
    }

    ret=writeFileHeader(oufd);
    if(ret<0){
        errProcess ("writeFileHeader",ret);
        exit(0);
    }

    ret=writeCompressArguments (oufd);
    if(ret<0){
        errProcess ("writeCompressArguments",ret);
        exit(0);
    }
    close(oufd);
    return 0;
}


int threadBlkInit(Stream_t *streamPtr,int index){
    int ret;
    streamPtr->workState=workState?DECPRESS:COMPRESS;
    streamPtr->blkSiz100k=blockSiz;
    streamPtr->treeShape=shapeMap(treeType);
    streamPtr->nodeCode=nodeCodeTypeMap(nodeCode);
    strcpy(streamPtr->infileName,fileInfo.orgfileName);


    sprintf (streamPtr->oufileName,"%s.%d",fileInfo.zipFileName,index);
    streamPtr->infd=open(streamPtr->infileName,O_RDONLY);//以只读方式打开文件
    if(streamPtr->infd<0){
        printf("open error\n");
        exit(0);
    }
    streamPtr->oufd=open(streamPtr->oufileName,O_WRONLY|O_CREAT,0777);//最后一个参数是指定权限
    if(streamPtr->oufd<0){
        printf("open error\n");
        exit(0);
    }

    streamPtr->curBlkSeq=0;

    streamPtr->blkOrigSiz=0;
    streamPtr->blkAfterSiz=0;

    streamPtr->verboseLevel=verbose;

    streamPtr->myAlloc=malloc;
    streamPtr->myFree=free;

    streamPtr->inbuff=(uchar*)streamPtr->myAlloc(streamPtr->blkSiz100k\
                                            *100000+sizeof(uchar)+OVERSHOOT
                                         );//OVERSHOOT=1000 wzip.h define定义
    if(streamPtr->inbuff==NULL){
        return ERR_MEMORY;
    }

    streamPtr->outbuff=(uchar*)streamPtr->myAlloc(streamPtr->blkSiz100k\
        *100000*2
        );
    memset( streamPtr->outbuff,0,streamPtr->blkSiz100k *100000*2);
    if(streamPtr->outbuff==NULL){
        return ERR_MEMORY;
    }
    //创建后缀数组,按照原来块大小的100000来创建
    streamPtr->suffixArray=(u32*)streamPtr->myAlloc(streamPtr->blkSiz100k\
                                            *100000*sizeof(u32)+sizeof(u32)
                                            );
    if(streamPtr->suffixArray==NULL){
        return ERR_MEMORY;
    }

    streamPtr->bwt=(uchar *)streamPtr->myAlloc(streamPtr->blkSiz100k\
        *100000+sizeof(uchar)
        );
    if (streamPtr->bwt==NULL)
    {
        return ERR_MEMORY;
    }

    memset(streamPtr->charMap,0,sizeof(streamPtr->charMap));
    memset(streamPtr->charFreq,0,sizeof(streamPtr->charFreq));
    memset(streamPtr->codeTable,0,CHAR_SET_SIZE*CODE_MAX_LEN);

    //累计输入数据的上下界值
    streamPtr->totalInHig32=0;
    streamPtr->totalInLow32=0;

    streamPtr->totalOuHig32=0;
    streamPtr->totalOuLow32=0;
    //小波数的根结点
    streamPtr->root=NULL;

    //累积cpu时间
    streamPtr->accCpuTime=0;
    //累积字符bit数
    streamPtr->accBitsPerChar=-1;
    //累积算出的压缩率
    streamPtr->accRatio=-1;

    //very important
    if(lseek (streamPtr->infd,
                    threadInfos[index].fileOffset,
                            SEEK_SET
              )==-1
      ){
        printf("lseek error\n");
        exit(0);
    }
    return 0;
}
int getHBblockSize(char* buff, u32 &HB_blockSize, int  len)
{
    HB_blockSize=256;
    double runs = 0;
    double avRuns = 0;
    u32 i;
    for (i = 0; i<len; i++)
        if (buff[i] != buff[i + 1])
            runs++;
    avRuns = len / runs;
    int a = 0;
    int b = 0;
    int HBLevel = 1;
    if (HBLevel<0 || HBLevel >2)
    {
        errProcess("HBLevel error", -1);
        exit(0);
    }
    switch (HBLevel)
    {
    case 0:a = 2; b = 10; break;
    case 1:a = 4; b = 20; break;
    case 2:a = 10; b = 50; break;
    default:a = 4; b = 20; break;
    }

    if (avRuns<a)
        HB_blockSize = HB_blockSize * 1;
    else if (avRuns<b)
        HB_blockSize = HB_blockSize * 2;
    else
        HB_blockSize = HB_blockSize * 4;
    return 0;

}

void *childThread(void *arg){
    int ret;
    int index=(int)(arg);


    if(gettimeofday (&threadInfos[index].startTime,NULL)<0){
        printf("gettimeofday error\n");
        exit(0);
    }

    Stream_t stream;
    ret=threadBlkInit (&stream,index);
    if(ret<0){
        printf("threadBlkInit\n");
        exit(0);
    }

    u32 nread;
    int blkCount=0;
   
    while ( blkCount<threadInfos[index].nblocks)
    {
        blkCount++;//important
        nread=stream.blkSiz100k*100000;
        //ssize_t read[1]  (int fd, void *buf, size_t count);//读文件函数
        ret=read(stream.infd,stream.inbuff,nread);
        if(ret<0){
            printf("read error\n");
            exit(0);
        }
        nread=ret;


        stream.blkOrigSiz=nread+1;
        stream.inbuff[stream.blkOrigSiz-1]='\0';
        if (stream.blkOrigSiz==1)
        {
            break;
        }

        //while computing the suffix array,the lib may use global varbile
        ret=blockSort(stream.inbuff,
                        stream.suffixArray,
                            stream.blkOrigSiz,/*must be blkSize */
                                &(stream.bwtIndex)
                       );
        if (ret<0)
        {
            errProcess("blockSort",ret);
            exit(0);
        }

        ret=getBwtTransform(stream.inbuff,stream.suffixArray,
            stream.bwt,stream.blkOrigSiz
            );
        if (ret<0)
        {
            errProcess("getBwtTransform",ret);
            exit(0);
        }

        if(stream.nodeCode==HYBIRD)//HYBIRD
        {
            if(HBblockSize==0)
            {
                getHBblockSize((char*)stream.bwt, HBblockSize, nread);
            }
            //printf("\n\n\t >>>>>>>>>>>get HBblockSize=%ld\n",HBblockSize);

        }

        ret=treeCode(stream.bwt,stream.blkOrigSiz,
            stream.treeShape,&stream
            );
        if (ret<0)
        {
            errProcess("treeCode",ret);
            exit(0);
        }

        waveletTree root=createWaveletTree(stream.bwt,
            stream.blkOrigSiz,
            stream.codeTable
            );

        if (!root)
        {
            errProcess("createWaveletTree",ERR_MEMORY);
            exit(0);
        }

        stream.root=root;//set the element of stream
        //compress bits-vector of wavelet tree
        ret=compressWaveletTree(stream.root,stream.nodeCode,HBblockSize);
        if (ret<0)
        {
            errProcess("compressWaveletTree",ret);
            exit(0);
        }

        int zipLen=computeZipSizWaveletTree(stream.root);
        if (zipLen<0)
        {
            errProcess("computeZipSizWaveletTree",zipLen);
            exit(0);
        }

        if (stream.nodeCode == HYBIRD)
        {
            uchar bitsNum = 3;
            while (HBblockSize >> bitsNum)bitsNum++;
            //printf("")
             write(stream.oufd,&bitsNum,sizeof (uchar));
        }

        ret=writeBlkCharSetMap(&stream);
        if (ret<0)
        {
            errProcess("writeBlkCharSetMap",ret);
            exit(0);
        }

        ret=writeBlkCharCodeTable(&stream);
        if (ret<0)
        {
            errProcess("writeBlkCharCodeTable",ret);
            exit(0);
        }

        //printf("\n\t???????????????????? BwtIndex=%ld\n",stream.bwtIndex);
        ret=writeBlkBwtIndex(&stream);
        if (ret<0)
        {
            errProcess("writeBlkBwtIndex",ret);
            exit(0);
        }

        ret=writeBlkZipNodeWithPreorder(&stream);
        if (ret<0)
        {
            errProcess("writeBlkZipNodeWithPreorder",ret);
            exit(0);
        }


        stream.totalInLow32+=stream.blkOrigSiz;
        if (stream.totalInLow32<stream.blkOrigSiz)
        {
            //means totalInlow overflow
            stream.totalInHig32++;
        }

        stream.totalOuLow32+=zipLen;
        if (stream.totalOuLow32<zipLen)
        {
            //means totalOuLow overflow
            stream.totalOuHig32++;
        }
        ret=destroyWaveletTree(stream.root);
        if (ret<0)
        {
            errProcess("destroyWaveletTree",ret);
            exit(0);
        }
        stream.root=NULL;

    }

    ret=streamBlkCompressCleanUp(&stream);
    if (ret<0)
    {
        errProcess("streamBlkCleanUp",ret);
        exit(0);
    }

    if(gettimeofday (&threadInfos[index].endTime,NULL)<0){
        printf("gettimeofday error\n");
        exit(0);
    }
}

void compressMainThread(void){
    int ret;

    char fileName[MAX_FILE_LEN];


    if(gettimeofday (&(threadInfos[0].startTime),NULL)<0)
    {
        printf("gettimeofday error\n");
        exit(0);
    }

    ret=mainThreadCompressInit();
    if(ret<0){
        printf ("mainThreadCompress error\n");
        exit(0);
    }

    int i;

    for(i=1;i<=nthread;i++)
    {
        if((threadInfos[i].threadId=fork())
                <0
          ){
            printf ("fork error\n");
            exit(0);
        }else if(threadInfos[i].threadId==0){
            //child process
            childThread ((void*)i);
            exit(0);
        }

        //parent process
    }
    if(signal (SIGINT,sig_int_compress)==SIG_ERR){
        printf("signal error\n");
        exit(0);
    }
    for(i=1;i<=nthread;i++){
        if(waitpid (threadInfos[i].threadId,NULL,0)<0){
            printf ("waitpid error\n");
            exit(0);
        }
    }

    int zipfd=open(fileInfo.zipFileName,O_WRONLY|O_APPEND);
    if(zipfd<0){
        printf("open error\n");
        exit(0);
    }


    int tempfd;

    uchar buff[BUFF_SIZE];
    u32 nread;

    //writeZipInfo
    write(zipfd,&nthread,sizeof(int));//4
    struct stat statBuff;
    off_t offset=lseek (zipfd,0,SEEK_CUR)+\
                        nthread*(sizeof(off_t)+sizeof(u32));
    //printf("start pos %lld\n",lseek(zipfd,0,SEEK_CUR));
    char tempFile[256];
    for(i=1;i<=nthread;i++)
    {
        write(zipfd,&offset,sizeof(off_t));
        write(zipfd,&threadInfos[i].nblocks,sizeof(int));
        //printf("offset=%lld,blocks=%d\n",offset,threadInfos[i].nblocks);
        sprintf (tempFile,"%s.%d",fileInfo.zipFileName,i);
        if(stat(tempFile,&statBuff)<0){
            printf("stat error\n");
            exit(0);
        }
        //printf("file %s:size %lld\n",tempFile,statBuff.st_size);
        offset+=statBuff.st_size;
    }

    //printf("Merge files Start\n");
    for(i=1;i<=nthread;i++){
        sprintf (fileName,"%s.%d",fileInfo.zipFileName,i);

        //open file
        tempfd=open(fileName,O_RDONLY);
        if(tempfd<0){
            printf("open error\n");
            exit(0);
        }
        while (1){
            nread=sizeof(buff);
            ret=read(tempfd,buff,nread);
            if (ret<0)
            {
                errProcess("read error",ret);
                exit(0);
            }else if(ret==0)
            {
                //eof
                break;
            }
            write(zipfd,buff,ret);
        }

        //eof
        close(tempfd);
        //remove tempfile
        unlink (fileName);
    }


    writeFileEnd(zipfd);

    if(gettimeofday (&(threadInfos[0].endTime),NULL)<0)
    {
        printf("gettimeofday error\n");
        exit(0);
    }

    struct stat tempBuff;
    if(verbose==1){
        printf("compress done.\n");
    }else if (verbose==2){
        if(stat(fileInfo.zipFileName,&tempBuff)!=0){
            printf("stat error\n");
            exit(0);
        }
        printf("compress done.\n");
         printf("\tfileSize%ld\n",fileInfo.orgFileSize);
        printf("\tratio:%.3f%%,\tbits/char:%.3f\n",
                    tempBuff.st_size*100.0/fileInfo.orgFileSize,
                           tempBuff.st_size*8.0/fileInfo.orgFileSize
               );

        struct timeval tmpTime;
        tmpTime.tv_sec=threadInfos[0].endTime.tv_sec - \
                            threadInfos[0].startTime.tv_sec;
        tmpTime.tv_usec=threadInfos[0].endTime.tv_usec- \
                            threadInfos[0].startTime.tv_usec;
        if(tmpTime.tv_usec<0){
            tmpTime.tv_sec--;
            tmpTime.tv_usec+=1000000;
        }

        printf("real time of mainThread:%.3f(s)\n",
                    tmpTime.tv_sec+tmpTime.tv_usec/1000000.0
               );
    }

    //remove the orignal file
    if(!keepOrigFile){
        unlink (fileInfo.orgfileName);
    }

    close(zipfd);

}


void decompressChildProcess(int i,Stream_t *streamPtr)
{
    if(!streamPtr)
    {
        printf ("parameter error\n");
        exit(0);
    }
    close(streamPtr->infd);
    close(streamPtr->oufd);

    char outFileName[256];
    sprintf (outFileName,"%s.%d",streamPtr->oufileName,i);
    sprintf (streamPtr->oufileName,outFileName);

    streamPtr->infd=open(streamPtr->infileName,O_RDONLY);
    if(streamPtr->infd<0){
        printf("open error\n");
        exit(0);
    }

    streamPtr->oufd=open(streamPtr->oufileName,O_CREAT|O_WRONLY,0777);
    if(streamPtr->oufd<0){
        printf ("open error\n");
    }

    if(lseek(streamPtr->infd,
                threadInfos[i].fileOffset,
                    SEEK_SET
             )<0
      ){
        printf("lseek error\n");
        exit(0);
    }

    int count=0;
    int ret;
    while(count<threadInfos[i].nblocks)
    {
      
        count++;
        if (streamPtr->nodeCode == HYBIRD)
        {
            ret = paraseHBblockSize(streamPtr,HBblockSize);
        }
        ret=paraseBlkCharSetMap(streamPtr);
        if (ret<0)
        {
            //error
            errProcess("paraseBlkCharSetMap",ret);
            streamBlkCompressCleanUp(streamPtr);
            exit(0);
        }

        ret=paraseBlkCharCodeTable(streamPtr);
        if (ret<0)
        {
            errProcess("praseBlkCharCodeTable",ret);
            exit(0);
        }

        ret=paraseBlkBwtIndex(streamPtr);
         //printf("\n\t???????????????????? BwtIndex=%ld\n",streamPtr->bwtIndex);
        if (ret<0)
        {
            errProcess("paraseBlkBwtIndex",ret);
            exit(0);
        }

        streamPtr->root=genWavtreeWithCodeTable(streamPtr->codeTable);
        if (!streamPtr->root)
        {
            errProcess("genWavtreeWithCodeTable",ERR_MEMORY);
            exit(0);
        }

        ret=paraseBlkZipNodeWithPreorder(streamPtr,HBblockSize);//HBblockSize
        if (ret<0)
        {
            errProcess("paraseBlkZipNodeWithPreorder",ret);
            exit(0);
        }

        ret=genBwtWithWaveletTree(streamPtr->root,streamPtr);
        if (ret<0)
        {
            errProcess("generateBwtWithWaveletTree",ret);
            exit(0);
        }

        ret=genOrigBlkWithBwt(streamPtr->bwt,
                                streamPtr->blkOrigSiz,
                                    streamPtr->bwtIndex,
                                        streamPtr->inbuff
                                );
        if (ret<0)
        {
            errProcess("genOrigBlkWithBwt",ret);
            exit(0);
        }

        ret=streamWriteOrigBlk(streamPtr);
        if (ret<0)
        {
            errProcess("streamWriteOrigBlk",ret);
            exit(0);
        }

        //very important
        destroyWaveletTree(streamPtr->root);
        streamPtr->root=NULL;

    }
    streamBlkDecompressCleanUp(streamPtr);

}

void decompressMainThread(void){
    int ret;
    Stream_t stream;
    stream.workState=DECPRESS;

    strcpy (stream.infileName,fileInfo.orgfileName);

    if(gettimeofday (&threadInfos[0].startTime,NULL)!=0)
    {
        printf("gettimeofday error\n");
        exit(0);
    }
    ret=streamBlkDecompressInit(&stream);
    if (ret<0)
    {
        errProcess("streamBlkDecompressInit",ret);
        exit(0);
    }

    off_t pos;

    //
    read(stream.infd,&nthread,sizeof(int));
    int blocks;
    off_t offset;
    int i;
    for(i=1;i<=nthread;i++){
        read(stream.infd,&threadInfos[i].fileOffset,sizeof(off_t));
        read(stream.infd,&threadInfos[i].nblocks,sizeof(int));
        //printf ("offset=%lld,blocks=%d\n",
        //     threadInfos[i].fileOffset,threadInfos[i].nblocks);
    }

    //create child process
    for(i=1;i<=nthread;i++)
    {
        threadInfos[i].threadId=fork();
        if(threadInfos[i].threadId<0){
            printf ("fork error\n");
            exit(0);
        }else if(threadInfos[i].threadId==0){
            //child process
            decompressChildProcess (i,&stream);
            exit(0);
        }
       //parent process
    }
    streamBlkDecompressCleanUp(&stream);
    //wait for child process
    if(signal (SIGINT,sig_int_decompress)==SIG_ERR){
        printf("signal error\n");
        exit(0);
    }
    for(i=1;i<=nthread;i++){
        if(waitpid (threadInfos[i].threadId,NULL,0)<0){
            printf("waitpid error\n");
            exit(0);
        }
    }

    //merge for files
    char buff[BUFF_SIZE];
    char tempfile[256];
    int  tempfd;

    stream.oufd=open(stream.oufileName,O_WRONLY|O_CREAT,0666);
    if(stream.oufd<0){
        printf("open error\n");
        exit(0);
    }
    if(ftruncate (stream.oufd,0)<0){
        printf ("ftruncate error\n");
        exit(0);
    }

    for(i=1;i<=nthread;i++)
    {
        sprintf (tempfile,"%s.%d",stream.oufileName,i);
        tempfd=open(tempfile,O_RDONLY);
        if(tempfd<0){
            printf ("open error\n");
            exit(0);
        }
        while(1){
            ret=read(tempfd,buff,sizeof(buff));
            if(ret<0){
                printf ("read error\n");
                exit(0);
            }else if(ret==0){
                break;
            }

            write(stream.oufd,buff,ret);
        }

        //need to remove temp file
        unlink (tempfile);
    }


    if(!keepOrigFile){
        unlink (stream.infileName);
    }

    if(gettimeofday (&threadInfos[0].endTime,NULL)!=0)
    {
        printf("gettimeofday error\n");
        exit(0);
    }

    if(verbose==1){
        printf("decompress done.\n");
    }else if (verbose==2){
        printf("decompress done.\n");

        struct timeval tmpTime;
        tmpTime.tv_sec=threadInfos[0].endTime.tv_sec - \
                            threadInfos[0].startTime.tv_sec;
        tmpTime.tv_usec=threadInfos[0].endTime.tv_usec- \
                            threadInfos[0].startTime.tv_usec;
        if(tmpTime.tv_usec<0){
            tmpTime.tv_sec--;
            tmpTime.tv_usec+=1000000;
        }

        printf("real time of mainThread:%.3f(s)\n",
                    tmpTime.tv_sec+tmpTime.tv_usec/1000000.0
               );
    }
}
