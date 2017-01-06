#ifndef  _TEST_BAI_H
#define  _TEST_BAI_H
void showSAandBWT(char*T, int *SA, int *BWT, int len);
void showSAandBWT(char*T, int *SA, char*L, int len);
int BWT_tansform(char*T, int *SA, char*L, int len, int &bwt_i);
#endif