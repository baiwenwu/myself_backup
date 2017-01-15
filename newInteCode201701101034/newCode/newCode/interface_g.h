#ifndef _INTERFACE_G_H_
#define _INTERFACE_G_H_
#include"type.h"
uchar getOneUchar(uchar *src, u32 index);
void bitsCopy(uchar *dst, u32 dst_index, uchar * src,
		u32 src_index, u32 len);
uchar getNextOneBit(uchar *src, uchar offset);
uchar getMark2(uchar **src, uchar *offset);
void writeMark2(uchar **src, uchar *offset, uchar val);
u32 getRuns(uchar **src, uchar *offset);
int getBitsOfNum(unsigned x);//求log(x)向下取整
int getSubGP(u32 x);
extern uchar *EndWords;
#endif