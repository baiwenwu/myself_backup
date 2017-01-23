#pragma once
#include<iostream>
using namespace std;

typedef unsigned int u32;
typedef unsigned char uchar;

void showElisGammaCode(uchar *buf, int offset);
void showElisDeltaCode(uchar *buf, int offset);
int runLengthPlusOneCode(uchar *src, u32 bitsLen, uchar *dst);
int plusOneCode(u32 num, uchar **buffPPtr, uchar *offset);
void printBitsForArray(uchar * src, u32 index, u32 len);
