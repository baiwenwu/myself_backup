#include "fileProcess.h"
#include <stdlib.h>
#include <string.h>

FILE *	myFileSafeOpen(char *filePath, char *mode){
	FILE *tmp=NULL;
	//tmp = fopen(filePath, mode);
	return tmp;
}

int myFileClose(FILE *fptr){
	return fclose(fptr);
}

bool fileExist(char *filePath){
	ofstream fp;
	fp.open(filePath);
	if (!fp.is_open())
	{
		return false;
	}
	else
	{
		fp.close();
		return true;
	}
}

int	myFileSafeCreate(char *filePath){
	ofstream fp;
	if (!fileExist(filePath))
	{
		//will create new file
		fp.open(filePath, ios::out);
		fp.close();
		return 0;//create file ok
	}
	else{
		//means file already exist
		return -1;//create file failed
	}
}

int	myFileRead(FILE * fptr, u32 *nread, uchar *buff){
	u32 origNread = *nread;
	*nread = fread(buff, sizeof(uchar), *nread, fptr);
	if (origNread != *nread)
	{	//encounter eof or IO error
		if (feof(fptr) == 0){
			//IO error
			return ERR_IO;
		}
		//eof
	}
	return 0;
}

int myFileWrite(FILE* fptr, u32 *nwrite, uchar *buff){
	u32 origNwrite = *nwrite;
	*nwrite = fwrite(buff, sizeof(uchar), *nwrite, fptr);
	if (*nwrite != origNwrite){
		return ERR_IO;
	}
	return 0;
}


//for devices which do not support fseek,this function means nothing
u32 getFileSize(FILE *fptr){
	long savedPos = ftell(fptr);

	fseek(fptr, 0, SEEK_END);
	long fileSiz = ftell(fptr);

	fseek(fptr, savedPos, SEEK_SET);
	return fileSiz;
}


char* getFileSuffix(char *filePath, char *suffix, int suffixLen){
	char *pos = strrchr(filePath, '.');
	if (pos == NULL){
		return NULL; //find nothing
	}
	strcpy_s(suffix, FILE_NAME_LEN, pos);
	//strcpy(suffix, pos);
	return pos;
}


/*
* 定义压缩文件的名字
* 根据压缩的文件的名字确定解压/压缩后的文件的名字,workState是压缩/解压缩的标记量
*/
int filenameMap(char *inName, char *outName, Mode workState){
	char suffix[FILE_NAME_LEN];
	char *ret;
	switch (workState)
	{
	case COMPRESS:
		ret = getFileSuffix(inName, suffix, FILE_NAME_LEN);
		if (ret == NULL){
			//find no suffix
			strcpy_s(outName, FILE_NAME_LEN,inName);
			strcat_s(outName, FILE_NAME_LEN, ".wz");
			return 0;
		}

		//check suffix
		if (strcmp(suffix, ".tar") == 0){
			strcpy_s(outName, FILE_NAME_LEN, inName);
			ret = strrchr(outName, '.');
			*ret = '\0';
			strcat_s(outName, FILE_NAME_LEN, ".twz");
			return 0;
		}
		else{
			strcpy_s(outName, FILE_NAME_LEN, inName);
			strcat_s(outName, FILE_NAME_LEN, ".wz");
			return 0;
		}
		break;
	case DECPRESS:
		ret = getFileSuffix(inName, suffix, FILE_NAME_LEN);
		if (ret == NULL)
		{
			printf("Decompress: %s may be error wzip name \n",
				inName);
			return ERR_FILE_NAME;
		}

		//may have suffix
		if (strcmp(suffix, ".wz") == 0){
			strcpy_s(outName, FILE_NAME_LEN, inName);
			ret = strrchr(outName, '.');
			*ret = 0;
			return 0;
		}
		else if (strcmp(suffix, ".twz") == 0){
			strcpy_s(outName, FILE_NAME_LEN, inName);
			ret = strrchr(outName, '.');
			*ret = 0;
			strcat_s(outName, FILE_NAME_LEN, ".tar");
			return 0;
		}
		else{
			printf("Decompress:%s may be error wzip name \n",
				inName);
			return ERR_FILE_NAME;
		}
		break;
	default:
		printf("unknown workState\n");
		return ERR_PARAMETER;

	}

}