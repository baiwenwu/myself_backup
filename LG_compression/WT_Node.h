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
class WT_Node
{
	public:
		WT_Node(unsigned long long int * bitbuff,int bit_len,int level,int block_size=1024,unsigned char label='\0',uchar ** tables=NULL);
		//bit_len:0,1串的实际长度，单位bit
		//level:层数
		//block_size:块大小
		//label:当前节点代表的字符.只有叶节点的label域又意义.
		
		WT_Node(){};
		WT_Node(uchar ** tables):zerostable(tables[0]),R(tables[1]){}
		~WT_Node();

		int Rank(int pos);
		int Rank(int pos,int &bit);


		void Left(WT_Node * left);
		//设置左孩子
		
		WT_Node * Left(){return left;};
		//返回左孩子

		void Right(WT_Node * right);
		//...

		WT_Node * Right(){return right;};
		//...

		unsigned char Label();
		int Load(loadkit & s);
		int Save(savekit & S);
		int SizeInByte();
	private:
		uchar* zerostable;
		uchar *R;
		WT_Node(const WT_Node &);
		WT_Node & operator =(const WT_Node& right);
		void Coding();

		//得到存储在data中的0,1串中的第index位
		int GetBit(u64 * data,int index);
		//从buff保存的0.1串中，由index位置开始，返回后续bits位表示的
		//数值.

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
		//把u64类型的value拷贝到data串的index处.
		void BitCopy(u64 * temp,int &index,u64 value);
		
		int level;//该串的层数.
		
		unsigned char label;
		//只有叶节点又意义，表示该节点代表的字符

		unsigned long long int * data;
		//0,1串的压缩存储体.

		int bitLen;
		//0,1串的长度，单位bit。

		int memorysize;
		int block_size;
		int block_width;
		int super_block_size;

		WT_Node * left;
		WT_Node * right;

		InArray *superblock;//超快偏移量
		InArray *block;//块的偏移量
		InArray *coding_style;//每个块的编码方案.0:plain, 1:RLG0, 2:RLG1;
		
		//这是个工作变量.
		unsigned long long int * buff;
};

#endif







