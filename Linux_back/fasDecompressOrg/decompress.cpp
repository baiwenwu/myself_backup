#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
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
#include "global.h"

int paraseFileHeader(Stream_t *streamPtr)
{
	int ret;
    if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	uchar buff[56];
    //ret=fread(buff,sizeof(uchar),FILE_HEAD_LEN,zipFile);
    ret=read(streamPtr->infd,buff,FILE_HEAD_LEN);
    if (ret!=FILE_HEAD_LEN)
	{
		return ERR_IO;
	}

	if (strncmp((char*)buff,"wzip_h",FILE_HEAD_LEN)!=0)
	{
		//content is error
		return ERR_CRC_CHECK;
	}
	return 0;
}

int paraseFileTail(FILE *zipFile,Stream_t *streamPtr)
{
	int ret;
	if (!zipFile || !streamPtr)
	{
		return ERR_PARAMETER;
	}
	uchar buff[56];
	ret=fread(buff,sizeof(uchar),FILE_TAIL_LEN,zipFile);
	if (ret !=FILE_TAIL_LEN)
	{
		return ERR_IO;
	}
	
	if (!strncmp((char*)buff,"wzip_t",FILE_HEAD_LEN))
	{
		//content is error
		return ERR_CRC_CHECK;
	}
	return 0;
}

int paraseBlkSiz(Stream_t *streamPtr)
{
	int ret;
    if ( !streamPtr)
	{
		return ERR_PARAMETER;
	}

	uchar blkSiz;
    //ret=fread(&blkSiz,sizeof(uchar),1,zipFile);
    ret=read(streamPtr->infd,&blkSiz,sizeof(uchar));
    if (ret !=sizeof(uchar))
	{
		return ERR_IO;
	}

	if (blkSiz==0 || blkSiz>9)
	{
		return ERR_CRC_CHECK;
	}

	streamPtr->blkSiz100k=blkSiz;
	return 0;
}

int paraseNodeCodeType(Stream_t *streamPtr)
{
	int ret;
    if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	uchar codeType;
    //ret=fread(&codeType,sizeof(codeType),1,zipFile);
    ret=read(streamPtr->infd,&codeType,sizeof(uchar));
    if (ret!=sizeof(codeType))
	{
		return ERR_IO;
	}

	if (codeType>2)
	{
		return ERR_CRC_CHECK;
	}
	if(codeType==2)
	{
		streamPtr->nodeCode=HYBIRD;
	}
	else
	{
		streamPtr->nodeCode=codeType?RLE_DELTA:RLE_GAMA;
	}
	return 0;
}

int streamBlkDecompressInit(Stream_t *streamPtr)
{
	if (!streamPtr)
	{
		return NULL;
	}

	int ret;
	memset(streamPtr->oufileName,0,sizeof(streamPtr->oufileName));
	ret=filenameMap(streamPtr->infileName,
						streamPtr->oufileName,
							streamPtr->workState
					);

    //printf ("%s \tto\t%s\n",streamPtr->infileName,streamPtr->oufileName);
	if (ret<0)
	{
		return ERR_FILE_NAME;
	}


    //check hard-link
    struct stat statBuff;
    if(stat(streamPtr->infileName,&statBuff)<0){
        printf ("sat error\n");
        exit(0);
    }
    if(statBuff.st_nlink>1 &&
            !keepOrigFile
       ){
        printf("%s has more than one hard-links,you may need -k option\n",
               streamPtr->infileName
              );
        exit(0);
    }

    streamPtr->fileSize=statBuff.st_size;

    streamPtr->infd=open(streamPtr->infileName,O_RDONLY);
    if(streamPtr->infd<0)
    {
        printf ("open error\n");
        exit(0);
    }

    //check whether output file exist
    if(stat(streamPtr->oufileName,&statBuff)==0 &&
                !overWrite
       ){
        printf ("%s already exists ,you may need -f option\n",
                    streamPtr->oufileName
                );
        exit(0);
    }
    streamPtr->oufd=open(streamPtr->oufileName,O_WRONLY|O_CREAT,0777);
    if(streamPtr->oufd<0)
    {
        printf ("open error\n");
        exit(0);
    }

	streamPtr->curBlkSeq=0;

	
    ret=paraseFileHeader(streamPtr);
	if (ret<0)
	{
		printf("paraseFileHead error\n");
		return ERR_CRC_CHECK;
	}

    ret=paraseBlkSiz(streamPtr);
	if (ret<0)
	{
		printf("paraseBlkSiz error\n");
		return ERR_CRC_CHECK;
	}


    ret=paraseNodeCodeType(streamPtr);
	if (ret<0)
	{
		printf("paraseNodeCodeType error\n");
		return ERR_CRC_CHECK;
	}

	
	streamPtr->blkOrigSiz=0;
	streamPtr->blkAfterSiz=0;

	streamPtr->verboseLevel=3;
	
	streamPtr->myAlloc=malloc;
	streamPtr->myFree=free;

	streamPtr->inbuff=(uchar *)streamPtr->myAlloc(streamPtr->blkSiz100k\
													*100000+sizeof(uchar)
													);
	if (!streamPtr->inbuff)
	{
		return ERR_MEMORY;
	}

	streamPtr->outbuff=(uchar*)streamPtr->myAlloc(streamPtr->blkSiz100k\
														*100000*2
												 );
	if(streamPtr->outbuff==NULL){
		return ERR_MEMORY;
	}

	streamPtr->suffixArray=NULL;


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


	streamPtr->totalInHig32=0;
	streamPtr->totalInLow32=0;

	streamPtr->totalOuHig32=0;
	streamPtr->totalOuLow32=0;

	streamPtr->root=NULL;

	streamPtr->accCpuTime=0;
	streamPtr->accBitsPerChar=-1;
	streamPtr->accRatio=-1;
	
	return 0;
}

int streamBlkDecompressCleanUp(Stream_t *streamPtr)
{
    close(streamPtr->infd);
    close(streamPtr->oufd);

	if (streamPtr->inbuff)
	{
		free(streamPtr->inbuff);
		streamPtr->inbuff=NULL;
	}

	if (streamPtr->outbuff)
	{
		free(streamPtr->outbuff);
		streamPtr->outbuff=NULL;
	}


	if (streamPtr->suffixArray)
	{
		free(streamPtr->suffixArray);
		streamPtr->suffixArray=NULL;
	}

	if (streamPtr->bwt)
	{
		free(streamPtr->bwt);
		streamPtr->bwt=NULL;
	}

	if (streamPtr->root)
	{
		destroyWaveletTree(streamPtr->root);
		streamPtr->root=NULL;
	}

	return 0;
}

int streamBlkDecompressNew(Stream_t *streamPtr)
{
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	memset(streamPtr->charMap,0,sizeof(streamPtr->charMap));
	memset(streamPtr->codeTable,0,CHAR_SET_SIZE*CODE_MAX_LEN);
}
int paraseHBblockSize(Stream_t *streamPtr,u32 &HBblockSize)
{
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	uchar bitsNum;
	int ret=read(streamPtr->infd,&bitsNum,sizeof(uchar));
	if(ret!=sizeof(uchar))
	{
		return ERR_IO;
	}
	HBblockSize = 1;
	HBblockSize = HBblockSize << (bitsNum - 1);
	//printf("\t HBblockSize=%ld\n",HBblockSize);
 	return 0;
}

int paraseBlkCharSetMap(Stream_t *streamPtr)
{
    if (!streamPtr)
	{
		return ERR_PARAMETER;
	}

	int ret;
	uchar buff[32];

    ret=read(streamPtr->infd,buff,sizeof(buff));
    if (ret!=sizeof(buff))
	{
		return ERR_IO;
	}

	uchar *ptr=buff;
	uchar offset=0;
	int i;

	//
	int setSiz=0;
	memset(streamPtr->charMap,0,sizeof(streamPtr->charMap));
	for (i=0;i<CHAR_SET_SIZE;i++)
	{
		if (*ptr &(1<<(7-offset)))
		{
			streamPtr->charMap[i]=true;
			setSiz++;
		}

		if (++offset==8)
		{
			offset=0;
			ptr++;
		}
	}

	streamPtr->setSize=setSiz;
	return 0;
}

int paraseBlkCharCodeTable(Stream_t *streamPtr)
{
	int ret;
    if ( !streamPtr)
	{
		return ERR_PARAMETER;
	}

	uchar len;
	uchar buff[56];

	int i,j;
	int nbytes;

	uchar *ptr;
	uchar offset;

	memset(streamPtr->codeTable,0,CHAR_SET_SIZE*CODE_MAX_LEN);

	for (i=0;i<CHAR_SET_SIZE;i++)
	{
		if (streamPtr->charMap[i])
		{
            ret=read(streamPtr->infd,&len,sizeof(uchar));
            if (ret!=sizeof(uchar))
			{
				return ERR_IO;
			}
			nbytes=len/8+(len%8?1:0);

            ret=read(streamPtr->infd,buff,nbytes);
            if (ret!=nbytes)
			{
				return ERR_IO;
			}

			//compute the code
			ptr=buff;
			offset=0;
			for (j=0;j<len;j++)
			{
				if (*ptr&(1<<(7-offset)))
				{
					streamPtr->codeTable[i][j]='1';
				}else
				{
					streamPtr->codeTable[i][j]='0';
				}

				if (++offset==8)
				{
					offset=0;
					ptr++;
				}
			}

		}
	}

	return 0;
}

int paraseBlkBwtIndex(Stream_t *streamPtr)
{
    if (!streamPtr)
	{
		return ERR_PARAMETER;
	}

	int ret;
	u32 bwtIndex;

    ret=read(streamPtr->infd,&bwtIndex,sizeof(u32));
    if (ret != sizeof(u32))
	{
		return ERR_IO;
	}
	streamPtr->bwtIndex=bwtIndex;
	return 0;
}

waveletTree genWavtreeWithCodeTable(char (*codeTable)[CODE_MAX_LEN])
{
	int i,j;
	int len;
	waveletTree root=NULL;
	if (!codeTable)
	{
		return NULL;
	}

	root=(waveletNode_t*)malloc(sizeof(waveletNode_t));
	if (!root)
	{
		return NULL;
	}
	memset(root,0,sizeof(waveletNode_t));

	waveletNode_t *node;
	waveletNode_t *leftNode,*righNode;
	for (i=0;i<CHAR_SET_SIZE;i++)
	{
		len=strlen(codeTable[i]);
		if (len==0)
		{
			continue;
		}
		node=root;
		for (j=0;j<len;j++)
		{
			if (codeTable[i][j]=='0')
			{
				if (!node->leftChild)
				{
					leftNode=(waveletNode_t*)
						malloc(sizeof(waveletNode_t));
					if (!leftNode)
					{
						return NULL;
					}
					memset(leftNode,0,sizeof(waveletNode_t));
					node->leftChild=leftNode;
				}
				node=node->leftChild;
			}else
			{
				if (!node->righChild)
				{
					righNode=(waveletNode_t*)
								malloc(sizeof(waveletNode_t));
					if (!righNode)
					{
						return NULL;
					}

					memset(righNode,0,sizeof(waveletNode_t));
					node->righChild=righNode;
				}
				node=node->righChild;
			}
		}
		node->label=i;
	}

	return root;
}

int readZipNode(waveletTree root,int zipfd,
                    int maxBisLen,NodeCodeType nodeType,u32 HBblockSize)
{
	if (root->leftChild==NULL && root->righChild==NULL)
	{
		return 0;
	}

	//internal node
	int nbytes=maxBisLen/8+(maxBisLen%8?1:0);
	root->bitBuff=(uchar*)malloc(nbytes);
	memset(root->bitBuff,0,nbytes);
	if (!root->bitBuff){
		return ERR_MEMORY;
	}
	int ret;

	//read zipLen
             //ret=fread(&(root->zipLen),sizeof(u32),1,zipFile);
             ret=read(zipfd,&(root->zipLen),sizeof(u32));
             if (ret !=sizeof(u32))
	{
		return ERR_IO;
	}

	int zipBytes=root->zipLen/8+(root->zipLen%8?1:0);
	root->zipBuff=(uchar*)malloc(zipBytes);
	if (!root->zipBuff)
	{
		return ERR_MEMORY;
	}

	//read zipBuff
    //ret=fread(root->zipBuff,sizeof(uchar),zipBytes,zipFile);
              ret=read(zipfd,root->zipBuff,zipBytes);
              if (ret !=zipBytes)
	{
		return ERR_IO;
	}

	//decompress zipBuff to bitBuff
	if(nodeType==HYBIRD)
	{
		ret=deGppHybirdCode(root->zipBuff, root->zipLen,root->bitBuff, HBblockSize);
		if (ret<0)
		{
			errProcess("runLengthHybirdCode",ret);
			return ret;
		}
		root->bitLen=ret;
	}
	else if (nodeType==RLE_GAMA)
	{
		ret=runLengthGammaDecode(root->zipBuff, root->zipLen,root->bitBuff);
		if (ret<0)
		{
			errProcess("runLengthGammaCode",ret);
			return ret;
		}
		root->bitLen=ret;
	}else
	{
		ret=runLengthDeltaDecode(root->zipBuff,
									root->zipLen,
										root->bitBuff
								);
		if (ret<0)
		{
			errProcess("runLengthDeltaDecode",ret);
			return ret;
		}
		root->bitLen=ret;
	}

	if (root->leftChild)
	{
        ret=readZipNode(root->leftChild,zipfd,root->bitLen,nodeType,HBblockSize);
		if (ret<0)
		{
			return ret;
		}
	}
	if (root->righChild)
	{
        ret=readZipNode(root->righChild,zipfd,root->bitLen,nodeType,HBblockSize);
		if (ret<0)
		{
			return ret;
		}
	}

	return 0;
}

int paraseBlkZipNodeWithPreorder(Stream_t *streamPtr,u32 HBblockSize)
{
    if (!streamPtr )
	{
		return ERR_PARAMETER;
	}
	int ret;

    ret=readZipNode(streamPtr->root,streamPtr->infd,	streamPtr->blkSiz100k*100000+1,
		streamPtr->nodeCode,HBblockSize);
	if (ret<0)
	{
		return ret;
	}

    streamPtr->blkOrigSiz=streamPtr->root->bitLen;

	return 0;
}


int decompressInitWaveletTree(waveletTree root)
{
	if (!root)
	{
		return ERR_PARAMETER;
	}
	if (root->leftChild==NULL && root->righChild==NULL )
	{
		return 0;
	}

	root->ptr=root->bitBuff;
	root->offset=0;
	
	int ret;
	if (root->leftChild)
	{
		ret=decompressInitWaveletTree(root->leftChild);
		if (ret<0)
		{
			return ret;
		}
	}

	if (root->righChild)
	{
		ret=decompressInitWaveletTree(root->righChild);
		if (ret<0)
		{
			return ret;
		}
	}
}
int genBwtWithWaveletTree(waveletTree root,Stream_t *streamPtr)
{
	if (!root ||!streamPtr)
	{
		return ERR_PARAMETER;
	}

	int ret=decompressInitWaveletTree(root);
	if (ret<0)
	{
		errProcess("decompressInitWaveletTree",ret);
		return ret;
	}

	u32 i;
	waveletNode_t *node;
	for (i=0;i<root->bitLen;i++)
	{
		node=root;

        waveletNode_t *tempPtr;
        while (node->leftChild!=NULL || node->righChild!=NULL)
        {

            tempPtr=(  *(node->ptr)&(1<<(7-node->offset))
                     )?(node->righChild):(node->leftChild);

            if (++(node->offset)==8)
            {
                node->offset=0;
                node->ptr++;
            }

            node=tempPtr;
        }
		streamPtr->bwt[i]=node->label;
	}
	return 0;
}

int genOrigBlkWithBwt(uchar *bwt,u32 len,u32 bwtIndex,uchar *orig)
{
	if (!bwt || !len ||!orig)
	{
		return ERR_PARAMETER;
	}

	u32 charFreq[CHAR_SET_SIZE];
	u32 charTemp[CHAR_SET_SIZE];
	memset(charFreq,0,sizeof(charFreq));
	memset(charTemp,0,sizeof(charFreq));

	int i;
	for (i=0;i<len;i++)
	{
		charFreq[bwt[i]]++;
	}

	//accumulate  count
	for (i=1;i<CHAR_SET_SIZE;i++)
	{
		charFreq[i]+=charFreq[i-1];
	}

	u32 *mapLF=(u32*)malloc(sizeof(u32)*len);
	if (!mapLF)
	{
		return ERR_MEMORY;
	}
	//compute mapLF
	for (i=0;i<len;i++)
	{
        mapLF[i]=charTemp[bwt[i]]+(bwt[i]?charFreq[bwt[i]-1]:0);
		charTemp[bwt[i]]++;
	}

    //very important,LF 修正
	mapLF[bwtIndex]=0;
	for (i=0;i<bwtIndex;i++)
	{
		if (!bwt[i])
		{
			mapLF[i]++;
		}
	}

    u32 index=bwtIndex;
    for (i=len-1;i>=0;i--)
    {
        orig[i]=bwt[index];
        index=mapLF[index];
    }
	free(mapLF);
	return 0;
}

int streamWriteOrigBlk(Stream_t *streamPtr)
{
    if (!streamPtr || !streamPtr->inbuff)
	{
		return ERR_PARAMETER;
	}

	int ret;
    ret=write(streamPtr->oufd,
                streamPtr->inbuff,
                    streamPtr->blkOrigSiz-1
             );
    if (ret!=streamPtr->blkOrigSiz-1)
	{
		return ERR_IO;
	}

	return 0;
}

