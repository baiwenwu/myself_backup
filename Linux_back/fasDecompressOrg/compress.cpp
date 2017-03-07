#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include <unistd.h>


int streamBlkCompressCleanUp(Stream_t *streamPtr){
	if (streamPtr==NULL)
	{
		return ERR_PARAMETER;
	}


    close(streamPtr->infd);
    close(streamPtr->oufd);

	if (streamPtr->inbuff)
	{
		streamPtr->myFree(streamPtr->inbuff);
		streamPtr->inbuff=NULL;
	}
	if(streamPtr->outbuff)
	{
		streamPtr->myFree(streamPtr->outbuff);
		streamPtr->outbuff=NULL;
	}

	
	if (streamPtr->suffixArray)
	{
		streamPtr->myFree(streamPtr->suffixArray);
		streamPtr->suffixArray=NULL;
	}

	if (streamPtr->bwt)
	{
		streamPtr->myFree(streamPtr->bwt);
		streamPtr->bwt=NULL;
	}

	if (streamPtr->root)
	{
		//destroy wavelet tree
		streamPtr->root=NULL;
	}

	return 0;
}

int streamBlkCompressNew(Stream_t *streamPtr){
	streamPtr->blkOrigSiz=0;
	streamPtr->blkAfterSiz=0;

	if (!streamPtr->infile)
	{
		return ERR_PARAMETER;
	}
	if(!streamPtr->oufile){
		return ERR_PARAMETER;
	}

	if (!streamPtr->inbuff)
	{
		return ERR_PARAMETER;
	}
	if (!streamPtr->outbuff)
	{
		return ERR_PARAMETER;
	}

	if (!streamPtr->suffixArray)
	{
		return ERR_PARAMETER;
	}

	if (!streamPtr->bwt)
	{
		return ERR_PARAMETER;
	}

	memset(streamPtr->charMap,0,sizeof(streamPtr->charMap));
	memset(streamPtr->charFreq,0,sizeof(streamPtr->charFreq));
	memset(streamPtr->codeTable,0,CHAR_SET_SIZE*CODE_MAX_LEN);

	if (streamPtr->root)
	{
		//destroy wavelet tree
		streamPtr->root=NULL;
	}

	streamPtr->curRatio=-1;
	return 0;
}

int getBlockDataInfo(uchar *inbuff,u32 len, 
					bool *charMap,u32 *charFreq,
					u32* setSiz)
{
	if (!inbuff || !charMap ||!charFreq)
	{
		return ERR_PARAMETER;
	}

	u32 i;

	for (i=0;i<CHAR_SET_SIZE;i++)
	{
		charFreq[i]=0;
	}

	for (i=0;i<len;i++)
	{
		charFreq[inbuff[i]]++;
	}

	*setSiz=0;
	for (i=0;i<CHAR_SET_SIZE;i++)
	{
		if (charFreq[i])
		{
			(*setSiz)++;
			charMap[i]=true;
		}else{
			charMap[i]=false;
		}
	}

	return 0;
}

int treeCode(uchar *inbuff,u32 len,
					TreeType shape,Stream_t *streamPtr
			)
{
	int ret;
	if (!inbuff || !len || !streamPtr)
	{
		return ERR_PARAMETER;
	}
	switch (shape)
	{

	case HUFFMAN:
		huffmanTree hufTree;
		hufTree=createHuffTree(inbuff,len,streamPtr);
		if (!hufTree)
		{
			errProcess("createHuffTree",ERR_MEMORY);
			return -1;
		}
		ret=generateHuffCode(hufTree,streamPtr->codeTable);
		if (ret<0)
		{
			errProcess("generateHuffCode",ret);
			destroyHuffTree(hufTree);
			return ret;
		}
		destroyHuffTree(hufTree);
		return 0;
		break;
    case BALANCE:
        balanceTree balTree;
        balTree=createBalanceTree(inbuff,len,streamPtr);
        if (!balTree)
        {
            errProcess("createBalanceTree",ERR_MEMORY);
            return -1;
        }
        ret=generateBalCode(balTree,streamPtr->codeTable);
        if (ret<0)
        {
            errProcess("generateBalCode",ret);
            destroyBalTree(balTree);
            return -1;
        }

        destroyBalTree(balTree);
        return 0;
        break;
	case HU_TACKER:
		hutackerTree hutTree;
		hutTree=createHutackerTree(inbuff,len,streamPtr);
		if (!hutTree)
		{
			errProcess("createHutackerTree",ERR_MEMORY);
			return -1;
		}

		ret=generateHutackerCode(hutTree,streamPtr->codeTable);
		if (ret<0)
		{
			errProcess("generateHutackerCode",ret);
			destroyHutackerTree(hutTree);
			return ret;
		}
        destroyHutackerTree (hutTree);
		return 0;
		break;

	}


	return 0;

}

int writeFileHeader(int oufd)
{
    if (oufd<0)
	{
		return ERR_PARAMETER;
	}
	uchar buff[50]={WZIP_W,WZIP_Z,WZIP_I,WZIP_P,WZIP__,WZIP_H,'\0'};
    write(oufd,buff,strlen((char*)buff));
    return 0;
}

int writeFileEnd(int zipfd)
{
    if (zipfd<0)
	{
		return ERR_PARAMETER;
	}
	uchar buff[50]={WZIP_W,WZIP_Z,WZIP_I,WZIP_P,WZIP__,WZIP_T,'\0'};
    write(zipfd,buff,strlen((char*)buff));
    return 0;
}

int writeCompressArg(FILE *zipFile,Stream_t *streamPtr)
{
	if (!zipFile || !streamPtr)
	{
		return ERR_PARAMETER;
	}

	uchar blkSiz100k;
	uchar nodeType;

	blkSiz100k=streamPtr->blkSiz100k;
	nodeType  =streamPtr->nodeCode;
	fwrite(&blkSiz100k,sizeof(uchar),1,zipFile);
	fwrite(&nodeType,sizeof(uchar),1,zipFile);
	return 0;
}

int writeBlkCharSetMap(Stream_t *streamPtr)
{
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	uchar buff[CHAR_SET_SIZE/8];
	memset(buff,0,sizeof(buff));

	int i;
	int bytePos;
	int bitOff;

	for (i=0;i<CHAR_SET_SIZE;i++)
	{
		if (streamPtr->charMap[i])
		{
			bytePos=i/8;
			bitOff=i%8;
			buff[bytePos]|=1<<(7-bitOff);
		}
	}
    write(streamPtr->oufd,buff,sizeof(buff));
    return 0;
}

int writeBlkCharCodeTable(Stream_t *streamPtr)
{
	if (!streamPtr || !streamPtr->codeTable)
	{
		return ERR_PARAMETER;
	}


	int i;
	int j;
	uchar codeBuff[10];
	uchar len;
	uchar *ptr;
	uchar offset;
	int nbytes;
	for (i=0;i<CHAR_SET_SIZE;i++)
	{
		len=strlen(streamPtr->codeTable[i]);
		if (len>0)
		{
			memset(codeBuff,0,sizeof(codeBuff));
			codeBuff[0]=len;

			ptr=&codeBuff[1];
			offset=0;

			for (j=0;j<len;j++)
			{
				if (streamPtr->codeTable[i][j]=='1')
				{
					*ptr|=1<<(7-offset);
				}
				
				if (++offset==8)
				{
					offset=0;
					ptr++;
				}
			}

			nbytes=len/8+(len%8?1:0)+1;
            write(streamPtr->oufd,codeBuff,nbytes);
        }
	}
	return 0;
}

int writeBlkBwtIndex(Stream_t *streamPtr)
{
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	u32 index=streamPtr->bwtIndex;
    write(streamPtr->oufd,&index,sizeof(u32));
    return 0;
}


int writeZipNode(waveletTree root,int zipfd)
{
	int ret;
    if (!root )
	{
		return ERR_PARAMETER;
	}

	if (root->leftChild==NULL && root->righChild==NULL)
	{
		//leaf node
		return 0;
	}

	u32 bitsLen=root->zipLen;
	int nbytes=root->zipLen/8+(root->zipLen%8?1:0);

    write(zipfd,&bitsLen,sizeof(u32));
    write(zipfd,root->zipBuff,nbytes);

	if (root->leftChild)
	{
        ret=writeZipNode(root->leftChild,zipfd);
		if (ret<0)
		{
			errProcess("writeZipNode",ret);
			return ret;
		}
	}
	if (root->righChild)
	{
        ret=writeZipNode(root->righChild,zipfd);
		if (ret<0)
		{
			errProcess("writeZipNode",ret);
			return ret;
		}
	}

	return 0;
}
int writeBlkZipNodeWithPreorder(Stream_t *streamPtr)
{
	if (!streamPtr || !streamPtr->root)
	{
		return ERR_PARAMETER;
	}

    int ret=writeZipNode(streamPtr->root,streamPtr->oufd);
	
	return ret;
}
