#include "compress.h"
#include "errProcess.h"
int writeFileHeader(ofstream &oufd)
{
	if (oufd)
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
{////log201612261454
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
	{//log201612261530
		nbytes = root->headLen / 8 + (root->headLen & 8 ? 1 : 0);
		oufd.write((char*)&root->headLen, sizeof(u32));//write(zipfd, &root->headLen, sizeof(u32));
		oufd.write((char*)root->head, nbytes);//write(zipfd, root->head, nbytes);
	}
	nbytes = root->zipLen / 8 + (root->zipLen % 8 ? 1 : 0);
	oufd.write((char*)&bitsLen, sizeof(u32));//write(zipfd, &bitsLen, sizeof(u32));
	oufd.write((char*)root->zipBuff, nbytes);//write(zipfd, root->zipBuff, nbytes);

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

