#include "errProcess.h"
char* strncpy(char* dest, const char* src, int len)
{

	if (!dest  && !src)
	{
		cout << "function: parameters of strncpy is error!" << endl;
	}
	char* temp = dest;
	int i = 0;
	while (i++ < len && (*temp++ = *src++) != '\0');
	if (*(temp) != '\0')
		*temp = '\0';
	return dest;
}
int errCodeMap(int errCode, char *errInfo, int len){
	switch (errCode)
	{
	case ERR_PARAMETER:
		strncpy(errInfo, "ERR_PARAMETER", len);
		break;
	case ERR_CRC_CHECK:
		strncpy(errInfo, "ERR_CRC_CHECK", len);
		break;
	case ERR_FILE_NAME:
		strncpy(errInfo, "ERR_FILE_NAME", len);
		break;
	case ERR_IO:
		strncpy(errInfo, "ERR_IO", len);
		break;
	case ERR_MEMORY:
		strncpy(errInfo, "ERR_MEMORY", len);
		break;
	case ERR_PARASE_ARG:
		strncpy(errInfo, "PARASE_ARG", len);
		break;
	default:
		return -1;
		break;
	}

	return 0;
}
void errProcess(char *info, int errCode){
	char errInfo[256];
	if (errCodeMap(errCode, errInfo, sizeof(errInfo))<0)
	{
		printf("Unknown errCode(%d):%s\n",
			errCode, info
			);
		return;
	}
	printf("%s (%d) :%s\n", errInfo, errCode, info);
	return;
}