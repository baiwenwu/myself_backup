/*============================================
# Filename: BitMap.h
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description: Hybrid-bitmap,support rank(int pos) and rank(int pos,int &bit)
=============================================*/
#ifndef WT_NODE_H
#define WT_NODE_H
#include<string.h>
#include"loadkit.h"
#include"savekit.h"
#include"InArray.h"
#include"BaseClass.h"
#include<math.h>
#include<iostream>
using namespace std;
class BitMap
{
	public:
		BitMap(unsigned long long int * bitbuff,int bit_len,int level,int block_size=1024,unsigned char label='\0',uchar ** tables=NULL);
		//bit_len:0,1串的实际长度，单位bit
		//level:层数
		//block_size:块大小
		//label:当前节点代表的字符.只有叶节点的label域又意义.
		
		BitMap(){};
		BitMap(uchar ** tables):Z(tables[0]),R(tables[1]){}
		~BitMap();

		int Rank(int pos);
		int Rank(int pos,int &bit);
		void Rank(int pos_left,int pos_right,int &rank_left,int &rank_right);


		void Left(BitMap * left);
		//设置左孩子
		
		BitMap * Left(){return left;};
		//返回左孩子

		void Right(BitMap * right);
		//...

		BitMap * Right(){return right;};
		//...

		unsigned char Label();
		int Load(loadkit & s);
		int Save(savekit & S);
		int SizeInByte();
		int testSrcAndUnzip(u64 *  src ,u64 *uzip , u64  start , u64  end );
		void decodeAll1(u64 *unzipbuff , u64 deIndex);
		void decodePlain(u64 *zipbuff,u64 zipIndex , u64 *unzipbuff , u64 deIndex , u64 bitLen);
		void decodeGamma(u64 *zipbuff,u64 &zipIndex , u64 *unzipbuff , u64 deIndex ,uchar flag);
		//u64 * deCompress(u64 * zipbuff);
		u64 * deCompress(u64 * src , u64 * zipbuff);
	private:
		uchar* Z;
		//uchar* R1;
		//uchar* R2;
		//uchar* R3;
		//uchar* R4;
		uchar *R;
		BitMap(const BitMap &);
		BitMap & operator =(const BitMap& right);
		void Coding();

		//得到存储在data中的0,1串中的第index位
		int GetBit(u64 * data,int index);
		//从buff保存的0.1串中，由index位置开始，返回后续bits位表示的
		//数值.

		u16 Zeros(u16 x){return (Z[x>>8]==8)?Z[x>>8]+Z[(uchar)x]:Z[x>>8];}
		u64 GetBits(u64 * buff,int &index,int bits);
	
		//得到0的runs.
		int GetZerosRuns(u64 * buff,int &index);
		//gamma解
		

		int FixedDecode(u64 * buff,int &index);
		int GammaDecode(u64 * buff,int &index);
 		
		//得到0,1串中的runs长度，bit标示该runs是针对谁的
		int GetRuns(u64 * data,int &index,int &bit);
		//index
		void Append_g(u64 * temp,int &index,u32 value);
	//	void Append_f(u64 * temp,int &index,u32 value);
		//把u64类型的value拷贝到data串的index处.
		void BitCopy(u64 * temp,int &index,u64 value);

		int level;//该串的层数.
		unsigned char label;
		//只有叶节点又意义，表示该节点代表的字符

		unsigned long long int * data;
		//0,1串的压缩存储体.

		int bitLen;
		//源串0,1串的长度，单位bit。

        //memorysize is the size of data(0,1串的压缩存储体)
		int memorysize;
		int block_size;
		int block_width;
		int super_block_size;

		BitMap * left;
		BitMap * right;

		InArray *superblock;//超快偏移量
		InArray *block;//块的偏移量
//		InArray *superblock_rank;//超快的偏移量
//		InArray *block_rank;//块的偏移量
		InArray *coding_style;//每个块的编码方案.0:plain, 1:RLG0, 2:RLG1;
		
		//这是个工作变量.
		unsigned long long int * buff;
};

#endif







