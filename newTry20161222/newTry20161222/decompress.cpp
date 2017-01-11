#define _FILE_OFFSET_BITS 64
#include <sys/stat.h>  
#include"decompress.h"
#include"errProcess.h"
#include"rlElisCode.h"

int paraseFileHeader(Stream_t *streamPtr)
{
	int ret;
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	uchar buff[56];
	//ret=fread(buff,sizeof(uchar),FILE_HEAD_LEN,zipFile);
	streamPtr->infd.read((char*)buff, FILE_HEAD_LEN);
	ret = streamPtr->infd.gcount(); //read(streamPtr->infd, buff, FILE_HEAD_LEN);
	if (ret != FILE_HEAD_LEN)
	{
		return ERR_IO;
	}
	if (strncmp((char*)buff, "wzip_h", FILE_HEAD_LEN) != 0)
	{
		//content is error
		return ERR_CRC_CHECK;
	}
	return 0;
}
int paraseBlkSiz(Stream_t *streamPtr)
{
	int ret;
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}

	uchar blkSiz;
	streamPtr->infd.read((char*)&blkSiz, sizeof(uchar));
	ret = streamPtr->infd.gcount(); //read(streamPtr->infd, &blkSiz, sizeof(uchar));
	if (ret != sizeof(uchar))
	{
		return ERR_IO;
	}

	if (blkSiz == 0 || blkSiz>9)
	{
		return ERR_CRC_CHECK;
	}

	streamPtr->blkSiz100k = blkSiz;
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
	streamPtr->infd.read((char*)&codeType, sizeof(uchar));
	ret = streamPtr->infd.gcount();//read(streamPtr->infd, &codeType, sizeof(uchar));
	if (ret != sizeof(codeType))
	{
		return ERR_IO;
	}

	if (codeType>4)
	{
		return ERR_CRC_CHECK;
	}
	if (codeType > 2)
		streamPtr->nodeCode = codeType - 2 ? PLUSONE : HBRID;
	else
		streamPtr->nodeCode = codeType ? RLE_DELTA : RLE_GAMA;
	return 0;
}
int paraseBlkCharCodeTable(Stream_t *streamPtr)
{
	int ret;
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}

	uchar len;
	uchar buff[56];

	int i, j;
	int nbytes;

	uchar *ptr;
	uchar offset;

	memset(streamPtr->codeTable, 0, CHAR_SET_SIZE*CODE_MAX_LEN);

	for (i = 0; i<CHAR_SET_SIZE; i++)
	{
		if (streamPtr->charMap[i])
		{
			streamPtr->infd.read((char*)&len, sizeof(uchar));
			ret = streamPtr->infd.gcount();
			if (ret != sizeof(uchar))
			{
				return ERR_IO;
			}
			nbytes = len / 8 + (len % 8 ? 1 : 0);

			streamPtr->infd.read((char*)buff, nbytes);
			ret = streamPtr->infd.gcount();
			if (ret != nbytes)
			{
				return ERR_IO;
			}

			//compute the code
			ptr = buff;
			offset = 0;
			for (j = 0; j<len; j++)
			{
				if (*ptr&(1 << (7 - offset)))
				{
					streamPtr->codeTable[i][j] = '1';
				}
				else
				{
					streamPtr->codeTable[i][j] = '0';
				}

				if (++offset == 8)
				{
					offset = 0;
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

	streamPtr->infd.read((char*)&bwtIndex, sizeof(u32));
	ret = streamPtr->infd.gcount();
	if (ret != sizeof(u32))
	{
		return ERR_IO;
	}
	streamPtr->bwtIndex = bwtIndex;
	return 0;
}

void rlBitMapInit()
{
	u16 R1 = 0,//打头的数字
		R2 = 0,//可解码个数
		R3 = 0,//偏移量
		R4 = 0;//有效解码位数,暂且不用
	u16 mask = -1;
	u32 tmp = -1;
	RLBITMAP[0] = 16ULL << 24;
	for (u32 i = 1; i < (1ULL << 8); i++)
	{
		u32 j = 1ULL << 15;
		u32 num0 = 0;
		while (j)
		{
			if (i&j)
				if (num0)
				{
					RLBITMAP[i] = num0 << 24;
					break;
				}
			num0++;
			j = j >> 1;
		}
	}
	for (u32 i = (1ULL << 8); i < 65536; i++)
	{
		R1 = R2 = R3 = R4 = 0;
		u32 andx = 1ULL << 15;
		u32 num0 = 0;
		while (andx)
		{
			if (i&andx)
			{
				if (!R2)
				{
					R2 = (num0 << 1) + 1;
					R1 = i >> (16 - R2);
				}
				if ((16 - R4)>(num0 << 1))
				{
					R3++;
					R4 += (num0 << 1) + 1;
				}
				andx = andx >> (num0 + 1);
				num0 = 0;
			}
			else
			{
				num0++;
				andx = andx >> 1;
			}
		}
		RLBITMAP[i] += (R4 & 0xff);
		RLBITMAP[i] = (RLBITMAP[i] << 8) + (R3);
		RLBITMAP[i] = (RLBITMAP[i] << 8) + (R2);
		RLBITMAP[i] = (RLBITMAP[i] << 8) + (R1);
	}
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
	//if (stat(fileInfo.orgfileName, &fileInfo.fileStat)<0)
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

    streamPtr->infd.open(streamPtr->infileName,ios::in|ios::binary);
    if(!streamPtr->infd.is_open())
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
    streamPtr->oufd.open(streamPtr->oufileName,ios::out|ios::binary);
    if(!streamPtr->oufd.is_open())
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
	
	if (streamPtr->nodeCode == 2)
	{//为混合编码初始化加速表
		rlBitMapInit();
	}

	streamPtr->blkOrigSiz=0;
	streamPtr->blkAfterSiz=0;

	streamPtr->verboseLevel=3;
	
	streamPtr->myAlloc=malloc;
	streamPtr->myFree=free;

	streamPtr->inbuff=(uchar *)streamPtr->myAlloc(streamPtr->blkSiz100k\
		*BLOCKTIMES + sizeof(uchar));
	if (!streamPtr->inbuff)
	{
		return ERR_MEMORY;
	}

	streamPtr->outbuff=(uchar*)streamPtr->myAlloc(streamPtr->blkSiz100k\
		*BLOCKTIMES * 2);
	if(streamPtr->outbuff==NULL){
		return ERR_MEMORY;
	}

	streamPtr->suffixArray=NULL;


	streamPtr->bwt=(uchar *)streamPtr->myAlloc(streamPtr->blkSiz100k\
		*BLOCKTIMES + sizeof(uchar));
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

	//streamPtr->root=NULL;

	streamPtr->accCpuTime=0;
	streamPtr->accBitsPerChar=-1;
	streamPtr->accRatio=-1;
	return 0;
}
int paraseHBblockSize(Stream_t *streamPtr)
{
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	uchar nblocks;
	streamPtr->infd.read((char*)&nblocks, sizeof(uchar));
	streamPtr->HBblockSize = nblocks;
	streamPtr->HBblockSize = streamPtr->HBblockSize << 8;
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

	streamPtr->infd.read((char*)buff, sizeof(buff));
	ret = streamPtr->infd.gcount();
	if (ret != sizeof(buff))
	{
		return ERR_IO;
	}

	uchar *ptr = buff;
	uchar offset = 0;
	int i;
	int setSiz = 0;
	memset(streamPtr->charMap, 0, sizeof(streamPtr->charMap));
	for (i = 0; i<CHAR_SET_SIZE; i++)
	{
		if (*ptr &(1 << (7 - offset)))
		{
			streamPtr->charMap[i] = true;
			setSiz++;
		}

		if (++offset == 8)
		{
			offset = 0;
			ptr++;
		}
	}

	streamPtr->setSize = setSiz;
	return 0;
}
//waveletTree genWavtreeWithCodeTable(char(*codeTable)[CODE_MAX_LEN])
waveletTree genWavtreeWithCodeTable(char codeTable[CHAR_SET_SIZE][CODE_MAX_LEN])
{
	int i, j;
	int len;
	waveletTree root = NULL;
	if (!codeTable)
	{
		return NULL;
	}

	root = (waveletNode_t*)malloc(sizeof(waveletNode_t));
	if (!root)
	{
		return NULL;
	}
	memset(root, 0, sizeof(waveletNode_t));

	waveletNode_t *node;
	waveletNode_t *leftNode, *righNode;
	for (i = 0; i<CHAR_SET_SIZE; i++)
	{
		len = strlen(codeTable[i]);
		if (len == 0)
		{
			continue;
		}
		node = root;
		for (j = 0; j<len; j++)
		{
			if (codeTable[i][j] == '0')
			{
				if (!node->leftChild)
				{
					leftNode = (waveletNode_t*)
						malloc(sizeof(waveletNode_t));
					if (!leftNode)
					{
						return NULL;
					}
					memset(leftNode, 0, sizeof(waveletNode_t));
					node->leftChild = leftNode;
				}
				node = node->leftChild;
			}
			else
			{
				if (!node->righChild)
				{
					righNode = (waveletNode_t*)
						malloc(sizeof(waveletNode_t));
					if (!righNode)
					{
						return NULL;
					}

					memset(righNode, 0, sizeof(waveletNode_t));
					node->righChild = righNode;
				}
				node = node->righChild;
			}
		}
		node->label = i;
	}

	return root;
}

int readZipNode(waveletTree root, ifstream &zipfd,
	int maxBisLen, NodeCodeType nodeType)
{
	if (root->leftChild == NULL && root->righChild == NULL)
	{
		//leaf node
		return 0;
	}

	//internal node
	int nbytes = maxBisLen / 8 + (maxBisLen % 8 ? 1 : 0);
	//root->bitBuff = (uchar*)malloc(nbytes);
	root->bitBuff = new uchar[nbytes];
	if (!root->bitBuff)
	{
		return ERR_MEMORY;
	}
	int ret;

	//read zipLen
	//ret=fread(&(root->zipLen),sizeof(u32),1,zipFile);
	//cout << zipfd.tellg() << endl;
	zipfd.read((char*)&(root->zipLen), sizeof(u32));
	ret = zipfd.gcount();
	if (ret != sizeof(u32))
	{
		return ERR_IO;
	}

	int zipBytes = root->zipLen / 8 + (root->zipLen % 8 ? 1 : 0);
	//root->zipBuff = (uchar*)malloc(zipBytes);
	root->zipBuff = new uchar[zipBytes];
	if (!root->zipBuff)
	{
		return ERR_MEMORY;
	}

	//read zipBuff
	//ret=fread(root->zipBuff,sizeof(uchar),zipBytes,zipFile);
	zipfd.read((char*)root->zipBuff, zipBytes);
	ret = zipfd.gcount();
	if (ret != zipBytes)
	{
		return ERR_IO;
	}

	//decompress zipBuff to bitBuff
	if (nodeType == RLE_GAMA)
	{
		ret = runLengthGammaDecode(root->zipBuff,
			root->zipLen,
			root->bitBuff
			);
		if (ret<0)
		{
			errProcess("runLengthGammaCode", ret);
			return ret;
		}
		root->bitLen = ret;
	}
	else if (nodeType == RLE_DELTA)
	{
		ret = runLengthDeltaDecode(root->zipBuff,
			root->zipLen,
			root->bitBuff
			);
		if (ret<0)
		{
			errProcess("runLengthDeltaDecode", ret);
			return ret;
		}
		root->bitLen = ret;
	}
	else
	{
		ret = runLengthPlusOneDecode(root->zipBuff,
			root->zipLen,
			root->bitBuff
			);
		if (ret<0)
		{
			errProcess("runLengthDeltaDecode", ret);
			return ret;
		}
		root->bitLen = ret;
	}

	if (root->leftChild)
	{
		ret = readZipNode(root->leftChild, zipfd, root->bitLen, nodeType);
		if (ret<0)
		{
			return ret;
		}
	}
	if (root->righChild)
	{
		ret = readZipNode(root->righChild, zipfd, root->bitLen, nodeType);
		if (ret<0)
		{
			return ret;
		}
	}

	return 0;
}
int readZipNode_HB(waveletTree root, Stream_t *streamPtr,
	int maxBisLen)
{
	if (root->leftChild == NULL && root->righChild == NULL)
	{
		//leaf node
		return 0;
	}
	
	//internal node
	int ret;
	
	//for head
	root->head = new bitArray(maxBisLen, 3, streamPtr->HBblockSize);
	streamPtr->infd.read((char*)&root->head->bitLen, sizeof(u32));
	ret = streamPtr->infd.gcount();
	if (ret != sizeof(u32))
	{
		return ERR_IO;
	}
	int headBytes = root->head->bitLen / 8 + (root->head->bitLen % 8 ? 1 : 0);
	streamPtr->infd.read((char*)root->head->arr, headBytes);
	ret = streamPtr->infd.gcount();
	if (ret != headBytes)
	{
		return ERR_IO;
	}
	//for zipBuff
	int nbytes = maxBisLen / streamPtr->HBblockSize + (maxBisLen % 8 ? 1 : 0);
	root->bitBuff = new uchar[nbytes];
	if (!root->bitBuff) 
	{
		return ERR_MEMORY;
	}
	streamPtr->infd.read((char*)&(root->zipLen), sizeof(u32));
	ret = streamPtr->infd.gcount();
	if (ret != sizeof(u32))
	{
		return ERR_IO;
	}
	int zipBytes = root->zipLen / 8 + (root->zipLen % 8 ? 1 : 0);
	root->zipBuff = new uchar[zipBytes];
	if (!root->zipBuff)
	{
		return ERR_MEMORY;
	}
	streamPtr->infd.read((char*)root->zipBuff, zipBytes);
	ret = streamPtr->infd.gcount();
	if (ret != zipBytes)
	{
		return ERR_IO;
	}


	//decompress HYBIRD zipBuff to bitBuff
	//ret = runLengthGammaDecode(root->zipBuff,root->zipLen,root->bitBuff	);
	ret = hybirdDecode(root, streamPtr->HBblockSize);
	if (ret<0)
	{
		errProcess("DeHyBirdCode error!", ret);
		return ret;
	}
	root->bitLen = ret;
	root->bitLen = maxBisLen;
	
	if (root->leftChild)
	{
		ret = readZipNode_HB(root->leftChild, streamPtr, root->bitLen);
		if (ret<0)
		{
			return ret;
		}
	}
	if (root->righChild)
	{
		ret = readZipNode_HB(root->righChild, streamPtr, root->bitLen);
		if (ret<0)
		{
			return ret;
		}
	}

	return 0;

}
int paraseBlkZipNodeWithPreorder(waveletTree root,Stream_t *streamPtr)
{
	if (!streamPtr)
	{
		return ERR_PARAMETER;
	}
	int ret;
	if (streamPtr->nodeCode != HBRID)
	{
		ret = readZipNode(root, streamPtr->infd,
			streamPtr->blkSiz100k * BLOCKTIMES + 1,
			streamPtr->nodeCode
			);
	}
	else
	{//DECODEHYBIRDCODE
		ret= readZipNode_HB(root, streamPtr, streamPtr->blkSiz100k * BLOCKTIMES + 1);
	}
	if (ret<0)
	{
		return ret;
	}

	streamPtr->blkOrigSiz = root->bitLen;

	return 0;
}
int decompressInitWaveletTree(waveletTree root)
{
	if (!root)
	{
		return ERR_PARAMETER;
	}
	if (root->leftChild == NULL && root->righChild == NULL)
	{
		return 0;
	}

	root->ptr = root->bitBuff;
	root->offset = 0;

	int ret;
	if (root->leftChild)
	{
		ret = decompressInitWaveletTree(root->leftChild);
		if (ret<0)
		{
			return ret;
		}
	}

	if (root->righChild)
	{
		ret = decompressInitWaveletTree(root->righChild);
		if (ret<0)
		{
			return ret;
		}
	}
}
int genBwtWithWaveletTree(waveletTree root, Stream_t *streamPtr)
{
	if (!root || !streamPtr)
	{
		return ERR_PARAMETER;
	}

	int ret = decompressInitWaveletTree(root);
	if (ret<0)
	{
		errProcess("decompressInitWaveletTree", ret);
		return ret;
	}

	u32 i;
	waveletNode_t *node;
	for (i = 0; i<root->bitLen; i++)
	{
		node = root;

		waveletNode_t *tempPtr;
		while (node->leftChild != NULL || node->righChild != NULL)
		{

			tempPtr = (*(node->ptr)&(1 << (7 - node->offset))
				) ? (node->righChild) : (node->leftChild);

			if (++(node->offset) == 8)
			{
				node->offset = 0;
				node->ptr++;
			}

			node = tempPtr;
		}
		streamPtr->bwt[i] = node->label;
	}
	return 0;
}
int genOrigBlkWithBwt(uchar *bwt, u32 len, u32 bwtIndex, uchar *orig)
{
	if (!bwt || !len || !orig)
	{
		return ERR_PARAMETER;
	}

	u32 charFreq[CHAR_SET_SIZE];
	u32 charTemp[CHAR_SET_SIZE];
	memset(charFreq, 0, sizeof(charFreq));
	memset(charTemp, 0, sizeof(charFreq));

	int i;
	for (i = 0; i<len; i++)
	{
		charFreq[bwt[i]]++;
	}

	//accumulate  count
	for (i = 1; i<CHAR_SET_SIZE; i++)
	{
		charFreq[i] += charFreq[i - 1];
	}

	u32 *mapLF = (u32*)malloc(sizeof(u32)*len);
	if (!mapLF)
	{
		return ERR_MEMORY;
	}
	//compute mapLF
	for (i = 0; i<len; i++)
	{
		mapLF[i] = charTemp[bwt[i]] + (bwt[i] ? charFreq[bwt[i] - 1] : 0);
		charTemp[bwt[i]]++;
	}

	//very important,LF 修正
	mapLF[bwtIndex] = 0;
	for (i = 0; i<bwtIndex; i++)
	{
		if (!bwt[i])
		{
			mapLF[i]++;
		}
	}

	u32 index = bwtIndex;
	for (i = len - 1; i >= 0; i--)
	{
		orig[i] = bwt[index];
		index = mapLF[index];
	}
	free(mapLF);
	return 0;
}
int streamBlkDecompressCleanUp(Stream_t *streamPtr)
{
	streamPtr->infd.close();
	streamPtr->oufd.close();

	if (streamPtr->inbuff)
	{
		free(streamPtr->inbuff);
		streamPtr->inbuff = NULL;
	}

	if (streamPtr->outbuff)
	{
		free(streamPtr->outbuff);
		streamPtr->outbuff = NULL;
	}


	if (streamPtr->suffixArray)
	{
		free(streamPtr->suffixArray);
		streamPtr->suffixArray = NULL;
	}

	if (streamPtr->bwt)
	{
		free(streamPtr->bwt);
		streamPtr->bwt = NULL;
	}
	return 0;
}

int streamWriteOrigBlk(Stream_t *streamPtr)
{
	if (!streamPtr || !streamPtr->inbuff)
	{
		return ERR_PARAMETER;
	}

	int ret = streamPtr->blkOrigSiz - 1;
	streamPtr->oufd.write((char*)streamPtr->inbuff, streamPtr->blkOrigSiz - 1);
	//ret = streamPtr->oufd.tellp();
	if (ret != streamPtr->blkOrigSiz - 1)
	{
		return ERR_IO;
	}

	return 0;
}