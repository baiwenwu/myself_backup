#include<iostream>
#include"test_bai.h"
using namespace std;
void circleOutStr(char *str,int start,  int len)
{
	int i = start + 1;
	int x = str[start];
	if (x>32)
	{
		cout << str[start];
	}
	else
	{
		if (str[start] == '\0')
		{
			cout << "$";
		}
		else
		{
			cout << "#";
		}	
	}
	//cout << str[start];
	while ((i = (i % len)) != start)
	{
		x = str[i];
		if (x>32)
		{	
			cout << str[i];
		}
		else
		{
			if (str[i] == '\0')
			{
				cout << "$";
			}
			else
			{
				cout << "#";
			}
		}
		i++;
	}
}
void showSAandBWT(char*T,int *SA,char*L, int len)
{
	int slen = len;
	cout << "-------------------------------------------------------------" << endl;
	cout << "Ô´´®:" << T << endl;
	for (int i = 0; i < slen; i++)
	{
		cout << i << "\t";
		circleOutStr(T, i, slen);
		cout << "\t"<<SA[i]<<"\t";
		circleOutStr(T, SA[i], slen);
		cout << "\t" << L[i] << endl;
	}
	cout << "-------------------------------------------------------------" << endl;
	cout << endl;
}

int BWT_tansform(char*T, int *SA, char*L, int len, int &bwt_i)
{
	if (!T || !SA || !L || !len)
	{
		cout << "parameters are error!" << endl;
		return -1;
	}
	for (int i = 0; i < len; i++)
	{
		
		if (SA[i] == 0)
		{
			bwt_i = i;
			L[i] = T[len- 1];
		}
		else
		{
			L[i] = T[SA[i] - 1];
		}//L[i] = SA[i] ? T[SA[i] - 1] : T[len - 1];
	}
	return 0;
}
