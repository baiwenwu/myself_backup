/*============================================
# Filename: BitMap.cpp
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description: 
=============================================*/
#include"BitMap.h"
#include<math.h>
#include<iostream>

#include "statics.h"

using namespace std;
#define lookuptable
inline int popcnt(unsigned long long int x)
{
	x = x -((x & 0xAAAAAAAAAAAAAAAA)>>1);
	x = (x & 0x3333333333333333)+((x>>2) & 0x3333333333333333);
	x =((x+(x>>4)) & 0x0F0F0F0F0F0F0F0F);
	return (x*0x0101010101010101)>>56;
}

int blog(int x)
{
	int ans = 0;
	while(x>0)
	{
		ans++;
		x=x>>1;
	}
	return ans;
}
void printBits_u64(u64 x)
{
	u64 tmp=(1ULL<<63);
	while(tmp)
	{
		if(x&tmp)
		{
			cout<<"1";
		}
		else
		{
			cout<<"0";
		}
		tmp=(tmp>>1);
	}
}
BitMap::BitMap(unsigned long long int * bitbuff,int bit_len,int level,int block_size,unsigned char label,uchar ** tables)
{
	this->data = bitbuff;
	this->bitLen = bit_len;
	this->memorysize = 0;
	this->level = level;
	this->block_size = block_size;
	this->block_width = blog(block_size);
	this->super_block_size = 16*block_size;
	this->label = label;
	left=NULL;
	right=NULL;
	superblock =NULL;
	block=NULL;
	coding_style=NULL;
	R=NULL;
	Z=NULL;
	
	if(data!=NULL)
	{
		this->Z = tables[0];
		this->R = tables[1];
		Coding();
		buff =NULL;
	}

}
/*
*-----------kkzone-compress-------------
*直接在结点上进行数据压缩统计
*/
int BitMap::SizeInByte()
{

	int size = 0;
	if(data!=NULL)
	{
		//return bitLen/8;
		//size+= superblock->GetMemorySize();
		//size+= block->GetMemorySize();
		size+= coding_style->GetMemorySize();
		size+= memorysize;
	
	}
	return size;
}


void BitMap::Coding()
{
	int u64Len =0;
	if(bitLen%64 == 0)
		u64Len = bitLen/64;
	else
		u64Len = bitLen/64+1;
	u64 * temp = new u64[u64Len];//临时存储压缩后的字符串
	memset(temp,0,u64Len*8);
	
	int index = 0;
	int step1 = block_size*16;
	int step2 = block_size;
	//int block_width = blog(block_size);
	superblock = new InArray(2*(bitLen/step1)+2,blog(bitLen));
	block      = new InArray(2*(bitLen/step2)+2,blog(step1-step2));
	coding_style      = new InArray(bitLen/step2+1,3);

	int rank=0;
	int space=0;
	int bits =0;
	int firstbit;
	int rl_g=0;
	int runs = 0;
	int bit=0;
	int * runs_tmp = new int[block_size];
	int k=0;
	int index2=0;
	int pre_space =0 ;

	superblock->SetValue(0,0);
	superblock->SetValue(1,0);
	block->SetValue(0,0);
	block->SetValue(1,0);

	while(index < bitLen)
	{
		if(index == bitLen)
			break;
		rl_g = 0;
		bits = 0;
		firstbit = 0;
		runs = 0;
		firstbit = GetBit(data,index);
		memset(runs_tmp,0,block_size*4);//4=sizeof(int)
		k=0;
 		runs=0;
		while(bits < block_size && index < bitLen)
		{

			runs = GetRuns(data,index,bit);
			bits = bits +runs;
			if(bit ==1)
				rank=rank+runs;
			runs_tmp[k] = runs;
			k++;
		}
		
		if(bits > block_size)
		{//调整成一个确定的块长
			int step =0;
			index = index -(bits - block_size);
			step = block_size+runs-bits;
			if(bit ==1)
				rank = rank -runs+step;
			runs_tmp[k-1] = step;
		}

		for(int i=0;i<k;i++)
			rl_g = rl_g + 2*blog(runs_tmp[i])-1;//算出所有的runsGamma值

		int thred=20;//原程序的句子
		// int thred=0;
		int len = min(rl_g,block_size-thred);//原程序中的句子
		if(k==1)
		{//ALL0和ALL1编码
			if(firstbit==0)
				coding_style->SetValue((index-1)/block_size,3);
			else
				coding_style->SetValue((index-1)/block_size,4);
			space = space +0;
		}
		else if(len == (block_size-thred) || index == bitLen)//plain
		{
			coding_style->SetValue((index-1)/block_size,2);
			int j=0;
			int num=0;
			if(index == bitLen)
			{
				space = space + bits;
				j = (index-bits)/64;
				num = bits%64?bits/64+1:bits/64;
			}
			else
			{
				space = space + block_size;
				j = (index - block_size)/64;
				num = block_size/64;
			}
			for(int kk=0;kk<num;kk++,j++)
				BitCopy(temp,index2,data[j]);
		}
		else
		{//rl_gamma
			if(firstbit == 0)
				coding_style->SetValue((index-1)/block_size,0);
			else
				coding_style->SetValue((index-1)/block_size,1);
			space =space + rl_g;
			for(int i=0;i<k;i++)
			{
				//cout<<runs_tmp[i]<<endl;
				Append_g(temp,index2,runs_tmp[i]);
			}
		}
	}

	/***********kkzone-bai-li statics***********************************/
	Statics::block_size=this->block_size;
	Statics::superblock_size=this->super_block_size;
	int32_t coding_len = coding_style->GetNum();

	for(int32_t i = 0, j; i < coding_len; i++){
		j = coding_style->GetValue(i);
		Statics::coding_styles[j]++;
	}


	int32_t bit_zero = 0, bit_one = 0;
	int32_t bitVal;
	for(int32_t i = 0 ; i < bitLen; i++){
		bitVal = GetBit(data, i);
		if(bitVal == 1){
			bit_one++;
			if(bit_zero > 0){
				Statics::runs_num[bit_zero]++;
				bit_zero = 0;
			}
		} else {
			bit_zero++;
			if(bit_one > 0){
				Statics::runs_num[bit_one]++;
				bit_one = 0;
			}
		}
	}
	if(bit_one > 0){
		Statics::runs_num[bit_one]++;
		bit_one = 0;
	}
	if(bit_zero > 0){
		Statics::runs_num[bit_zero]++;
		bit_zero = 0;
	}



	//ave runs in blocks;
	for(int32_t i = 0, j; i < coding_len; i++){
		j = coding_style->GetValue(i);
		if(j == 0 || j == 1){ //run length 0/1
			//Static::numOfRlg0_1++;
			int32_t k = i * block_size;
			int32_t cnt = 0;
			bit_zero = 0, bit_one = 0;
			for(; cnt < block_size && cnt + k < bitLen; cnt++){
				bitVal = GetBit(data, cnt + k);

				if(bitVal == 1){
					bit_one++;
					if(bit_zero > 0){
						Statics::runs_blocks[bit_zero]++;
						bit_zero = 0;
					}
				} else {
					bit_zero++;
					if(bit_one > 0){
						Statics::runs_blocks[bit_one]++;
						bit_one = 0;
					}
				}
			}//end-of-for
			if(bit_one > 0){
				Statics::runs_blocks[bit_one]++;
				bit_one = 0;
			}
			if(bit_zero > 0){
				Statics::runs_blocks[bit_zero]++;
				bit_zero = 0;
			}

		}//end-of-if
	}//end-of-for
	//get the size of everyPats;
	Statics::headerSize+=coding_style->GetMemorySize();
	Statics::BSize+=block->GetMemorySize();
	Statics::SBSize+=superblock->GetMemorySize();
	/* finish the static for HEADER */
	for(int32_t i=0,j;i<coding_len;i++)
	{
		j = coding_style->GetValue(i);
		if(j==2)//2 represent plain coding style
			Statics::plainSize+=block_size/8;//block_size divides 8 can let us get the numbers of byte of a block;  
	}
	/***********kkzone-bai-li statics***********************************/	
	//释放runs_tmp
	delete [] runs_tmp;
	int u64_len_real = 0;
	if(space % 64==0)
		u64_len_real = space /64+1;
	else
		u64_len_real = space /64 +1+1;
	
	this->memorysize = u64_len_real*8;
	/***********kkzone-bai-li statics***********************************/	
	Statics::SSize +=this->memorysize;
	/***********kkzone-bai-li statics***********************************/
	//----------------------kkzone-compress-----------------
	cout<<"-------------------------------------------------------------------------------------"<<endl;
	//deCompress( data , temp );
	u64* unzipBits = deCompress( data , temp );
	int mark_i=0;
	int errorNum = 0 ;
	u32 bitWords_t=bitLen>>6;
	for(int i = 0 ; i < bitWords_t ; i++)
	{
		if(data[i]!=unzipBits[i])
		{
			if(!mark_i)
			{
				mark_i=i;
			}
			errorNum++;
		}
	}
	if(bitLen&0x3f)
	{
		//bitWords_t;
		u32 x= bitLen&0x3f;
		u32 y=64-x;
		if((data[bitWords_t]>>y)!=(unzipBits[bitWords_t]>>y))
		{
			cout<<"警告,原串和解压串的边界发生错误"<<endl;
		}
	}
	if(errorNum)
	{
		cout<<"第一个出错位置:\tmark_i="<<mark_i<<endl;
		cout<<"解压总数是:"<< u64Len<<"\t解压错误个数:"<<errorNum<<endl;
		cout<<"bitLen="<<bitLen<<"\t 出错bits="<<mark_i*64<<endl;
	}
	else
	{
		cout<<"解压总数是:"<< u64Len<<"\t解压错误个数:"<<errorNum<<endl;
	}
	
	
	//----------------------kkzone-compress-----------------
	delete [] data;
	data = new u64[u64_len_real];

	memset(data,0,u64_len_real*8);
	memcpy(data,temp,(u64_len_real-1)*8);
	delete [] temp;

}

BitMap::~BitMap()
{
	if(left)
		delete left;
	if(right)
		delete right;
	delete [] data;
	delete superblock;
	delete block;
	delete coding_style;
}

int BitMap::GetBit(u64 * data,int index)
{
	int anchor = index/64;
	int pos = 63-index%64;
	return ((data[anchor] &(0x01ull<<pos))>>pos);
}


//2014.5.8:16:53:这三段程序的性能相当.
int BitMap::GetRuns(u64 * data,int &index,int &bit)
{


	bit = GetBit(data,index);
	index = index +1;
	int totle_runs = 1;
	int runs=0;
	
	while(totle_runs < block_size)
	{
		u16 x= GetBits(data,index,16);//index不联动
		if(bit==1)
			x=(~x);
		runs = Zeros(x);
		totle_runs +=runs;
		index+=runs;
		if(runs < 16)
			break;
	}
	return totle_runs;
}


//gamma编码,index联动
void BitMap::Append_g(u64 *temp,int &index,u32 value)
{
	u64 y=value;
	int zerosnum = blog(value)-1;
	index+=zerosnum;
	int onesnum = zerosnum+1;
	if(index%64 + onesnum < 65)
	{
		temp[index/64] = (temp[index/64] | (y<<(64-(index%64 + onesnum))));
	}
	else
	{
		int first = 64 - index%64;
		int second = onesnum - first;
		temp[index/64] = (temp[index/64] | (y>>second));
		temp [index/64 +1] = (temp[index/64+1] | (y<<(64-second)));
	}
	index = index + onesnum;
}



void BitMap::BitCopy(u64 * temp,int & index,u64 value)
{
	if(index%64!=0)
	{
		int first = 64 - index % 64;
		int second = 64 - first;
		temp[index/64] = (temp[index/64] | (value>>second));
		temp[index/64 + 1] = (temp[index/64+1] | (value<<first));
	}
	else
		temp[index/64]  = value;
	index = index +64;
}

int BitMap::GammaDecode(u64 * buff,int & index)
{
	u32 x = GetBits(buff,index,32);//读取从index位置开始的32bits数据
	int runs = Zeros(x>>16);
	int bits = (runs<<1)+1;
	index = index + bits;
	return x>>(32-bits);
}


//从buff的index位置开始,读取bits位数据,返回.
u64 BitMap::GetBits(u64 * buff,int &index,int bits)
{

	if((index & 0x3f) + bits < 65)
		return (buff[index>>6]<<(index &0x3f))>>(64-bits);

	int first = 64 - (index &0x3f);
	int second = bits - first;
	u64 high = (buff[index>>6] & ((0x01ull<<first)-1)) << second;
	return high + (buff[(index>>6)+1]>>(64-second));
}

int BitMap::GetZerosRuns(u64 * buff,int &index)
{
	
	u32 x = GetBits(buff,index,16);
	int runs = Zeros(x);
	index = index + runs;
	return runs;

}


void BitMap::Left(BitMap * left)
{
     this->left = left;
}


void BitMap::Right(BitMap * right)
{
     this->right = right;
}
 
unsigned char BitMap::Label()
{
    return label;
}


int BitMap::Load(loadkit & s)
{
	s.loadi32(level);
	s.loadu8(label);
	s.loadi32(bitLen);
	s.loadi32(block_size);
	block_width = blog(block_size);
	s.loadi32(super_block_size);
	s.loadi32(memorysize);
	this->data=NULL;
	this->superblock=NULL;
	this->block=NULL;
	this->coding_style=NULL;
	if(memorysize!=0)
	{
		//we find memorysize represent the size of this->data array
		this->data = new u64[memorysize/8];
		s.loadu64array(data,memorysize/8);
	
		superblock = new InArray();
		superblock->load(s);
		
		block = new InArray();
		block->load(s);
		
		coding_style = new InArray();
		coding_style->load(s);
	}
    return 0;
}


int BitMap::Save(savekit & s)
{
	s.writei32(level);
	s.writeu8(label);
	s.writei32(bitLen);
	s.writei32(block_size);
	s.writei32(super_block_size);
	s.writei32(memorysize);
	if(memorysize!=0)
	{
		s.writeu64array(data,memorysize/8);
	
		superblock->write(s);
		block->write(s);
		coding_style->write(s);
	}
	return 0;
}
//---------------------------kkzone-compress------------------------
int BitMap::testSrcAndUnzip(u64 *  src ,u64 *uzip , u64  start , u64  end )
{
	if(!src || !uzip)
	{
		cout<<"测试解压是否正确接口传入参数有失误! BitMap.cpp : 566 行!"<<endl;
	}
	if(end>bitLen)
	{// 边界检查
		end=bitLen;
	}
	int errorNum=0;
	u64 sWords= start>>6;
	u64 eWords= end>>6;
	
	//cout<<eWords<<endl;
	for( u64 i=sWords;i<eWords;i++)
	{
		if(src[i] != uzip[i])
		{
			cout<<i<<"\t(";
			printBits_u64(src[i] );
			cout<<")"<<endl;
			cout<<i<<"\t_";
			printBits_u64(uzip[i] );
			cout<<"_"<<endl;
			errorNum++;
		}
		else
		{
			//cout<<i<<"\t(";
			//printBits_u64(src[i] );
			//cout<<")"<<endl;
		}
	}
	if(end&0x3f)
	{
		eWords++;
		u32 x= end&0x3f;
		u32 y=64-x;
		if((src[eWords]>>y)!=(uzip[eWords]>>y))
		{
			cout<<"警告,原串和解压串的边界发生错误"<<endl;
		}
	}
	return errorNum;
}
void writeG1ToUnzipbuff(u64 *unzipbuff , u64 deIndex,u64 num1)
{//there have a bug ,2016/9/20,the bug has been solved

	int words=deIndex >> 6 ;
	u64 offset =deIndex & 0x3f;
	int  tmp = num1>>6;
	u64 tmpOffset= num1 & 0x3f;
	u64 all1_64=-1;
	// if(words>5790)
	// {
	// 	cout<<words<<"\t";
	// 	printBits_u64(unzipbuff[words] );
	// 	cout<<endl;
	// }
	for(int i = 0 ; i < tmp ; i++ )
	{
		unzipbuff[words++]+=(all1_64>>offset);
		if(offset) 
		{//修改bug3
			unzipbuff[words]+=(all1_64<<(64-offset));
		}	
	}
	u64 hyBits=(offset+tmpOffset)&0x3f;
	if(tmpOffset)
	{
		if(((offset+tmpOffset)>>6)&&hyBits)
		{
			u64 hyBits=(offset+tmpOffset)&0x3f;
			unzipbuff[words++]+=(all1_64>>offset);
			unzipbuff[words]+=(all1_64<<(64-hyBits));
		}
		else
		{
			u64 tmpAll64=all1_64<<(64-tmpOffset);
			unzipbuff[words]+=(tmpAll64>>offset);
		}	
	}
	// cout<<words<<"\t";
	// printBits_u64(unzipbuff[words] );
	// cout<<endl;
	
}
void BitMap::decodeAll1(u64 *unzipbuff,u64 deIndex)
{
	if(!unzipbuff)
	{
		cout<<"解压缩参数错误(BitMap.cpp, 506行)!";
	}
	int unzWords=deIndex>>6;
	u64 unzOffset = deIndex & 0x3f;
	u64 tmp=-1;
	if(bitLen-deIndex<block_size)
	{
		int num1=bitLen-deIndex;
		writeG1ToUnzipbuff(unzipbuff , deIndex,num1);
		return;
	}
	if( unzOffset )
	{//采用定长块编码,解压串不会出现解压时偏移量不等于0的情况,除非rlg解压过程中会有这种情况出现
		cout<<"解压All1时参数出错,BitMap.cpp,521 行"<<endl;
		unzipbuff[unzWords++] += ( tmp>>unzOffset ) ;
		u64 y=block_size - ( 64 - unzOffset);
		u64 yOffset = y & 0x3f;
		for(u64 i =0;i<( y >> 6);i++)
		{
			unzipbuff[unzWords++] = tmp ;
		}
		unzipbuff[unzWords++] = tmp << ( 64 - yOffset );
	}
	else
	{
		for(u64 i = 0; i < ( block_size >> 6 ); i++)
		{
			unzipbuff[unzWords++] = tmp;
		}
	}	
}
inline u64 getValue64(u64 * temp , u64 index){
	int words=index>>6;
	u64 offset=index & 0x3f;
	if(offset)
	{
		return ( temp[words] << offset ) + ( temp[words+1] >> ( 64 - offset ) ) ;
	}
	else
	{
		return temp[words];
	}
}
void BitMap::decodePlain(u64 *zipbuff,u64 zipIndex , u64 *unzipbuff , u64 deIndex , u64 bitLen)
{
	int words=deIndex>>6;
	if(words>3120)
	{
		int x=0;
		int y=0;
		int z=x+y;	
	}
	if(deIndex & 0x3f)
	{
		cout<<cout<<"解压Plain时参数出错,BitMap.cpp,541 行"<<endl;
	}
	if(bitLen- deIndex<block_size)//出现隐式类型转换警告
	{
		int bits= bitLen - deIndex;
		//int num = (bits & 0x3f) ? ( (bits >> 6)+1) : (bits >> 6);
		int num = bits>>6;
		for(int i = 0; i< num ; i++ )
		{
			unzipbuff[ words++ ] = getValue64( zipbuff , zipIndex);
			zipIndex += 64 ;
		}
		int offsetBits=bits&0x3f;
		u64 tmp=getValue64( zipbuff , zipIndex);
		tmp=tmp>>(64-offsetBits);
		unzipbuff[ words] =(tmp<<(64-offsetBits));
	}
	else
	{
		for(int i = 0; i< (block_size>>6) ; i++ )
		{
			unzipbuff[ words++ ] = getValue64( zipbuff , zipIndex);
			zipIndex += 64 ;
		}
	}
	
}

void BitMap::decodeGamma(u64 *zipbuff,u64 &zipIndex , u64 *unzipbuff , u64 deIndex ,uchar flag)
{
	int decodeBits=0;
	while(decodeBits < block_size)
	{
		if((deIndex>>6)>2264)
		{
			int x=0;
			int y=0;
			int z = x + y;
		}
		//u64 BitMap::GetBits(u64 * buff,int &index,int bits)
		int index=zipIndex;
		u32 x = this->GetBits( zipbuff , index , 32);//读取从index位置开始的32bits数据
		//u32 x =0 ;
		int runs = Zeros(x>>16);
		int bits = (runs<<1)+1;
		//index = index + bits;
		u32 num01=x>>(32-bits);
		if(flag)
		{//调用写入1的函数
			writeG1ToUnzipbuff( unzipbuff , deIndex, num01) ;
		}
		zipIndex += bits;
		decodeBits+= num01;
		deIndex+=num01;
		flag=!flag;
	}
}
u64 * BitMap::deCompress(u64 * src , u64 * zipbuff)
{
	int u64Len =0;
	if(bitLen%64 == 0)
		u64Len = bitLen/64;
	else
		u64Len = bitLen/64+1;
	u64 * unzipbuff = new u64[u64Len];//临时存储解压后的字符串
	memset(unzipbuff,0,sizeof(u64)*u64Len);
	u64 block_i=0;
	u64 deIndex=0;
	u64 zipIndex=0;

	while(deIndex< bitLen)
	{
		int errorNum =0;
		switch(coding_style->GetValue(block_i++))
		{
			case 0: 
			{
				//cout<<" 0 : RL-Gamma0编码"<<endl;
				decodeGamma(zipbuff,zipIndex , unzipbuff , deIndex ,0);
				errorNum=testSrcAndUnzip( src, unzipbuff, deIndex, deIndex+block_size);//testSrcAndUnzip(u64 *  src ,u64 *uzip , int  start , int  end )
				if(errorNum)
				{
					cout<<" 0 : RL-Gamma0编码"<<endl;
					cout<<" zipIndex = "<<zipIndex<<endl;
					cout<<"errorNum = "<< errorNum<<endl;
					cout<<"------------------------------------------kkzone-compress-test-------------------------------------"<<endl;
				}
				deIndex+=block_size;	
			}
			break;
			case 1:
			{
				decodeGamma(zipbuff,zipIndex , unzipbuff , deIndex ,1);
				//errorNum=testSrcAndUnzip( src, unzipbuff, deIndex, deIndex+block_size);//testSrcAndUnzip(u64 *  src ,u64 *uzip , int  start , int  end )
				if(errorNum)
				{
					cout<<" 1 : RL-Gamma1编码"<<endl;
					cout<<" zipIndex = "<<zipIndex<<endl;
					cout<<"errorNum = "<< errorNum<<endl;
					cout<<"------------------------------------------kkzone-compress-test-------------------------------------"<<endl;
				}
				deIndex+=block_size;
			}
			break;
			case 2://调用plain解码函数
			{
				decodePlain(zipbuff , zipIndex ,  unzipbuff ,  deIndex, bitLen );
				//errorNum=testSrcAndUnzip( src, unzipbuff, deIndex, deIndex+block_size );//testSrcAndUnzip(u64 *  src ,u64 *uzip , int  start , int  end )
				if(errorNum)
				{
					 cout<<" 2 : Plain编码"<<endl;
					 cout<<" zipIndex = "<<zipIndex<<endl;
					 cout<<"errorNum = "<< errorNum<<endl;
					 cout<<"------------------------------------------kkzone-compress-test-------------------------------------"<<endl;
				}
				zipIndex+=block_size;
				deIndex+=block_size;
				break;
			}			
			case 3:
			{
				//errorNum=testSrcAndUnzip( src, unzipbuff, deIndex, deIndex+block_size );//testSrcAndUnzip(u64 *  src ,u64 *uzip , int  start , int  end )
				if(errorNum)
				{
					cout<<" 0 : All0编码"<<endl;
					cout<<" zipIndex = "<<zipIndex<<endl;
					cout<<"errorNum = "<< errorNum<<endl;
					cout<<"------------------------------------------kkzone-compress-test-------------------------------------"<<endl;
				}
				deIndex+=block_size;
				break;
			}
			case 4:
			{
				decodeAll1(unzipbuff , deIndex);
				//errorNum=testSrcAndUnzip( src, unzipbuff, deIndex, deIndex+block_size);
				if(errorNum)
				{
					cout<<" 2 : All1编码"<<endl;
					cout<<" zipIndex = "<<zipIndex<<endl;
					cout<<"errorNum = "<< errorNum<<endl;
					cout<<"------------------------------------------kkzone-compress-test-------------------------------------"<<endl;
				}
				deIndex+=block_size;
			}
			break;
			default:
			cout<<"some thing is wrong for deCompress!"<<endl;
		}
		if(errorNum)
		{
			exit(1);
		}
		
	}
	return unzipbuff;
}
//---------------------------kkzone-compress------------------------
