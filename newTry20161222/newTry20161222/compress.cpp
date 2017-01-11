#include "compress.h"
#include "errProcess.h"

int getBlockDataInfo(uchar *inbuff, u32 len,bool *charMap, u32 *charFreq,u32* setSiz)
{//log201612291103
	if (!inbuff || !charMap || !charFreq)
	{
		return ERR_PARAMETER;
	}
	u32 i;
	for (i = 0; i<CHAR_SET_SIZE; i++)
	{
		charFreq[i] = 0;
	}
	for (i = 0; i<len - 1; i++)
	{
		charFreq[inbuff[i]]++;
	}
	*setSiz = 0;
	for (i = 0; i<CHAR_SET_SIZE; i++)
	{
		if (charFreq[i])
		{
			(*setSiz)++;
			charMap[i] = true;
		}
		else{
			charMap[i] = false;
		}
	}
	return 0;
}
int getBlockSizeForHybirdCode(uchar *inbuff, u32 len, u32 &BlockSize, uchar level){
	int ret = 0;
	if (!inbuff)
	{
		return ERR_PARAMETER;
	}
	double runs = 0;
	double avRuns = 0;
	u32 i;
	for (i = 0; i<len - 1; i++)
		if (inbuff[i] != inbuff[i + 1])
			runs++;
	avRuns = len / runs;
	int a = 0;
	int b = 0;
	level = 1;
	BlockSize = 256;
	if (level<0 || level >2)
	{
		errProcess("speedlevel error", -1);
		exit(0);
	}
	switch (level)
	{
	case 0:a = 2; b = 10; break;
	case 1:a = 4; b = 20; break;
	case 2:a = 10; b = 50; break;
	default:a = 4; b = 20; break;
	}

	if (runs<a)
		BlockSize = BlockSize * 1;
	else if (runs<b)
		BlockSize = BlockSize * 2;
	else
		BlockSize = BlockSize * 4;
	return ret;
}


int writeFileHeader(ofstream &oufd)
{
	if (!oufd)
	{
		return ERR_PARAMETER;
	}
	uchar buff[50] = { WZIP_W, WZIP_Z, WZIP_I, WZIP_P, WZIP__, WZIP_H, '\0' };
	//log201612261334
	oufd.write((char *)buff, strlen((char*)buff));//write(const unsigned char *buf, int num);
	return 0;
}
int writeFileEnd(ofstream &zipfd)
{//
	uchar buff[50] = { WZIP_W, WZIP_Z, WZIP_I, WZIP_P, WZIP__, WZIP_T, '\0' };
	zipfd.write((char*)buff, strlen((char*)buff));//write(zipfd, buff, strlen((char*)buff));
	return 0;
}
int writeBlkCharSetMap(ofstream &oufd, const bool *charMap)
{//log201612261454
	if (!charMap)
	{
		return ERR_PARAMETER;
	}
	uchar buff[CHAR_SET_SIZE / 8];//CHAR_SET_SIZE=256
	memset(buff, 0, sizeof(buff));

	int i;
	int bytePos;
	int bitOff;

	for (i = 0; i<CHAR_SET_SIZE; i++)
	{
		if (charMap[i])
		{
			bytePos = i / 8;
			bitOff = i % 8;
			buff[bytePos] |= 1 << (7 - bitOff);
		}
	}
	oufd.write((char *)buff, sizeof(buff));//write(streamPtr->oufd, buff, sizeof(buff));
	//cout << "\t" << "2." << "1" << ".1";
	//cout << "写入字符表：" << buff << endl;
	return 0;
}
int writeBlkCharCodeTable(ofstream &oufd, char codeTable[][CODE_MAX_LEN])
{//log201612261511
	if (!codeTable)
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
	for (i = 0; i<CHAR_SET_SIZE; i++)
	{
		len = strlen(codeTable[i]);
		if (len>0)
		{
			memset(codeBuff, 0, sizeof(codeBuff));
			codeBuff[0] = len;

			ptr = &codeBuff[1];
			offset = 0;

			for (j = 0; j<len; j++)
			{
				if (codeTable[i][j] == '1')
				{
					*ptr |= 1 << (7 - offset);
				}

				if (++offset == 8)
				{
					offset = 0;
					ptr++;
				}
			}
			nbytes = len / 8 + (len % 8 ? 1 : 0) + 1;
			oufd.write((char*)codeBuff, nbytes);//write(streamPtr->oufd, codeBuff, nbytes);
		}
	}
	return 0;
}
int writeBlkBwtIndex(ofstream &oufd,const u32 bwtIndex)
{//log201612261518
	oufd.write((char *)&bwtIndex, sizeof(u32));//write(streamPtr->oufd, &index, sizeof(u32));
	return 0;
}
int writeZipNode(waveletTree root, ofstream &oufd)
{//log201612261530
	int ret;
	if (!root)
	{
		return ERR_PARAMETER;
	}

	if (root->leftChild == NULL && root->righChild == NULL)
	{//leaf node
		return 0;
	}
	u32 bitsLen = root->zipLen;
	int nbytes;
	if (root->head)
	{//log201612261530//-----baiwenwu-back------------
		u32 HeadLen = root->head->getBitsLenofArray();
		nbytes = HeadLen / 8 + (HeadLen % 8 ? 1 : 0);
		oufd.write((char*)&HeadLen, sizeof(u32));//write(zipfd, &root->headLen, sizeof(u32));
		oufd.write((char*)root->head->arr, nbytes);//write(zipfd, root->head, nbytes);
	}
	nbytes = root->zipLen / 8 + (root->zipLen % 8 ? 1 : 0);
	//char *ch_tmp = "baiwenwu";
	//oufd.write((char*)ch_tmp, strlen(ch_tmp));
	//cout << "\t\t\t起始位置5.1：" << oufd.tellp() << endl;
	oufd.write((char*)&bitsLen, sizeof(u32));//write(zipfd, &bitsLen, sizeof(u32));
	//cout << "\t\t\t起始位置5.2：" << oufd.tellp() << endl;
	oufd.write((char*)root->zipBuff, nbytes);//write(zipfd, root->zipBuff, nbytes);
	//cout << oufd.tellp() << endl;
	if (root->leftChild)
	{
		ret = writeZipNode(root->leftChild, oufd);
		if (ret<0)
		{
			errProcess("writeZipNode", ret);
			return ret;
		}
	}
	if (root->righChild)
	{
		ret = writeZipNode(root->righChild, oufd);
		if (ret<0)
		{
			errProcess("writeZipNode", ret);
			return ret;
		}
	}

	return 0;
}
int writeBlkZipNodeWithPreorder(ofstream &oufd, const waveletTree root)
{//log201612261527
	if (!root)
	{
		return ERR_PARAMETER;
	}

	int ret = writeZipNode(root,oufd);

	return ret;
}

