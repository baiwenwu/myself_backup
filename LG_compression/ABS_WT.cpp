#include"ABS_WT.h"
#include"statics.h"
#include<string.h>
#include<map>

u64 GetBits(u64 * buff,int &index,int bits)
{
	if((index & 0x3f) + bits < 65)
		return (buff[index>>6] >>( 64 -((index&0x3f) + bits))) & ((0x01ull<<bits)- 1);
	int first = 64 - (index &0x3f);
	int second = bits - first;
	u64 high = (buff[index>>6] & ((0x01ull<<first)-1)) << second;
	return high + (buff[(index>>6)+1]>>(64-second));
}

int Zeros(u16 x,ABS_FM *t)
{
	if(t->Z[x>>8]==8)
		return t->Z[x>>8]+t->Z[(uchar)x];
	else
		return t->Z[x>>8];
}

int GammaDecode(u64 * buff,int & index,ABS_FM * t)
{
	u32 x = GetBits(buff,index,32);
	int runs = Zeros(x>>16,t);
	int bits = (runs<<1)+1;
	index = index + bits;
	return x>>(32-bits);
}

ABS_FM::ABS_FM(const char * filename,int block_size,int D)
{
	this->block_size = block_size;
	this->D =D;
	this->T=NULL;
	T = Getfile(filename);
	Inittable();
}

ABS_FM::~ABS_FM()
{
	DestroyWaveletTree();
	if(T)
		delete [] T;
	if(bwt)
		delete [] bwt;
	if(SAL)
		delete SAL;
	if(RankL)
		delete RankL;
	if(C)
		delete [] C;
	if(code)
		delete [] code;
	if(Z)
		delete [] Z;
	if(R)
		delete [] R;
}
/*
*------------kkzone-compress--
*获取压缩率的接口
*/
int ABS_FM::SizeInByte()
{
	return TreeSizeInByte(root);
	//return TreeSizeInByte(root) + SAL->GetMemorySize() + RankL->GetMemorySize();
}

int ABS_FM::SizeInByte_count()
{
	return TreeSizeInByte(root);
}

int ABS_FM::TreeNodeCount(BitMap * r)
{
	if(r==NULL)
		return 0;
	return TreeNodeCount(r->Left()) + TreeNodeCount(r->Right()) + 1;
}

int ABS_FM::TreeSizeInByte(BitMap * r)
{
	int size = 0;
	if(r->Left())
		size += TreeSizeInByte(r->Left());
	if(r->Right())
		size+=TreeSizeInByte(r->Right());
	size = size + r->SizeInByte();
	return size;
}


unsigned char * ABS_FM::Getfile(const char *filename)
{
	FILE * fp = fopen(filename,"r+");
	if(fp==NULL)
	{
		cout<<"Be sure the file is available"<<endl;
		exit(0);
	}
	fseek(fp,0,SEEK_END);
	this->n = ftell(fp)+1;
	unsigned char * T = new unsigned char[n];
	fseek(fp,0,SEEK_SET);

	int e=0;
	int num=0;
	while((e=fread(T+num,sizeof(uchar),n-1-num,fp))!=0)
		num = num +e;
	if(num!=n-1)
	{
		cout<<"Read source file failed"<<endl;
		exit(0);
	}
	T[n-1]=0;
	fclose(fp);

	memset(charFreq,0,256*sizeof(int));
	memset(charMap,0,256*sizeof(bool));
	for(int i=0;i<n;i++)
		charFreq[T[i]]++;
	this->alphabetsize = 0;
	for(i32 i=0;i<256;i++)
		if(charFreq[i])
		{
			this->alphabetsize++;
			this->charMap[i]=true;
		}
	this->code = new int[256]; 

	this->C = new int[alphabetsize+1];
	memset(C,0,(alphabetsize+1)*4);
	this->C[alphabetsize] = n;
	this->C[0] = 0;
	int k=1;
	i32 pre =0;
	for(int i=0;i<256;i++)
	{
		if(charFreq[i]!=0)
		{
			code[i]=k-1;
			C[k]=pre + charFreq[i];
			pre = C[k];
			k++;
		}
		else
			code[i]=-1;
	}
	return T;
}


int ABS_FM::BWT(unsigned char *T,int * SA,unsigned char * bwt,int len)
{
	int i=0;
	int index=0;
	for(i=0;i<len;i++)
	{
		index = (SA[i]-1+len)%len;
		bwt[i]=T[index];
	}
	return 0;
}

/*
*-----------kkzone-compress---------
*构造SA
*构造BWT变换字符串
*/
int ABS_FM::BuildTree(int speedlevel)
{
	int *SA = new int[n];
	divsufsort(T,SA,n);		
	//SA和Rank数组的采样
	Statics::fileSize_kkz=n;
	int step1 =this->D;
	int step2 =this->D*16;
	//SA逆和SA数组的大小
	SAL=new InArray(n/step1+1,blog(n));
	RankL=new InArray(n/step2+1,blog(n));

	int i=0;
	int j=0;
	for(i=0,j=0;i<n;i=i+step1,j++)
		SAL->SetValue(j,SA[i]);
	
	for(i=0;i<n;i++)
	{
		if(SA[i]==0)
			continue;
		if((n-2-(SA[i]-1))%step2 == 0)
		{
			RankL->SetValue((n-2-(SA[i]-1))/step2,i);
		}
	}

	bwt = new unsigned char[n];
	BWT(T,SA,bwt,n);
	
	double runs=0.0;
	for(int i=0;i<n-1;i++)
		if(bwt[i]!=bwt[i+1])
			runs++;
	runs=n/runs;
	//-------kkzone-bai
	Statics::aveRunsOfL=runs;
	
	int a=0;
	int b=0;
	if(speedlevel<0 || speedlevel >2)
	{
		cerr<<"speedlevel error"<<endl;
		exit(0);
	}
	switch(speedlevel)
	{
		case 0:a=2;b=10;break;
		case 1:a=4;b=20;break;
		case 2:a=10;b=50;break;
		default:a=4;b=20;break;
	}
	
	if(runs<a)
		block_size=block_size*1;
	else if(runs<b)
		block_size=block_size*2;
	else
		block_size=block_size*4;
	
//	cout<<"block_size: "<<block_size<<endl;
	TreeCode();
	root=CreateWaveletTree(bwt,n);
//	cout<<"CreatWaveletTree"<<endl;

	delete [] T;
	T=NULL;
	delete [] SA;
	SA=NULL;
	delete[] bwt;
	bwt=NULL;

	return 0;
}

void ABS_FM::Test_Shape(BitMap * r)
{
	if(r->Left() && r->Right())
	{
		Test_Shape(r->Left());
		Test_Shape(r->Right());
	}
	else if(r->Left() || r->Right())
	{
		cout<<"one child"<<endl;
	}
}

/*
* -------kkzone-compress
* 构造小波树,实质是调用构造小波树的函数
*/
BitMap * ABS_FM::CreateWaveletTree(unsigned char * bwt,int n)
{
	BitMap * root = NULL;

	root = FullFillWTNode(bwt,n,0);
	if(!root)
	{
		cout<<"FullfillWTNode failed"<<endl;
		exit(0);
	}
	return root;
}

/* creat the wavelet tree */ 
/*
* -------kkzone-compress
* 构造小波树的函数
*/
BitMap * ABS_FM::FullFillWTNode(unsigned char * buff,int len,int level)
{
//	cout<<level<<endl;
	int CurrentLevel = level;
	unsigned int CurrentBitLen = len;
	unsigned char CurrentLabel = '\0';
	unsigned long long int *CurrentBitBuff = NULL;
	if ((int)strlen((const char*)codeTable[buff[0]])==level)
	{
		CurrentLabel = buff[0];
		CurrentBitBuff = NULL;
		//uchar * tables[5]={this->zerostable,this->R1,this->R2,this->R3,this->R4};
		uchar * tables[2] ={this->Z,this->R};
		BitMap * node = new BitMap(CurrentBitBuff,CurrentBitLen,CurrentLevel,block_size,CurrentLabel,tables);
		node->Left(NULL);
		node->Right(NULL);
		return node;
	}
	
	int u64Len=0;
	if(len%64==0)
		u64Len = len/64+1;
	else
		u64Len = len/64+2;
	CurrentBitBuff = new unsigned long long int[u64Len];
	memset(CurrentBitBuff,0,u64Len*8);
	unsigned char * lptr=NULL;
	unsigned char * rptr=NULL;
	int leftLen=0;
	int rightLen=0;

	lptr = new unsigned char[len];
	rptr = new unsigned char[len];
	memset(lptr,0,len);
	memset(rptr,0,len);

	//computer bitvect;

	int i=0;
	int bytePos=0;
	int bitOffset=0;
	u64 last = 0;
	for(i=0;i<len;i++)
	{
		if(codeTable[buff[i]][level]=='1')
		{
			//CurrentBitBuff[bytePos] |= (0x01<<(7-bitOffset));
			CurrentBitBuff[bytePos] |= (0x01ull<<(63-bitOffset));
			//construct right data buff
			rptr[rightLen++]=buff[i];
			last = 0;
		}
		else
		{
			lptr[leftLen++]=buff[i];
			last = 1;
		}
		bitOffset++;
		//if(bitOffset == 8)
		if(bitOffset == 64)
		{
			bytePos++;
			bitOffset = 0;
		}
	}
	CurrentBitBuff[bytePos] |= (last<<(63-bitOffset));

	//uchar * tables[5] = {this->zerostable,this->R1,this->R2,this->R3,this->R4};
	uchar * tables[2] = {this->Z,this->R};
	BitMap * node = new BitMap(CurrentBitBuff,CurrentBitLen,CurrentLevel,block_size,CurrentLabel,tables);

	if(leftLen !=0)
	{
		BitMap * left =FullFillWTNode(lptr,leftLen,level+1);
		node->Left(left);
		delete [] lptr;
		lptr=NULL;
	}
	if(rightLen!=0)
	{
		BitMap * right = FullFillWTNode(rptr,rightLen,level+1);
		node->Right(right);
		delete [] rptr;
		rptr=NULL;
	}
	return node;
}


int ABS_FM::DestroyWaveletTree()
{
	delete root ;
	root=NULL;
	return 0;
}


int ABS_FM::blog(int x)
{
	int ans=0;
	while(x>0)
	{
		ans++;
		x=(x>>1);
	}
	return ans;
}


void ABS_FM::Inittable()
{
	this -> Z = new uchar[1<<8];
	int tablewidth = 8;
	for(int i=0;i<tablewidth;i++)//i表示一个8bits里面从右往左第几位
		for(int j=(1<<i);j<(2<<i);j++)//从第i位到i+1位有多少个值,其前的0的个数相同,记录进Z[j] 
			Z[j] = tablewidth-1-i;
	Z[0] = tablewidth;
	
	u64 tablesize = (1<<16);
	R  = new uchar[tablesize<<2];//R中含有4个值分别是R1 R2 R3 R4
	
	//查找表的初始化：在16bits的0,1串上模拟gamma解码的过程，得到
	//这些表
	u64 B[2]={0xffffffffffffffffull,0xffffffffffffffffull};
	int sum =0;//gamma编码的和,含义为原串被编码的bits数目。
	int step=0;//16bits可以完整解码的bits数,该值不会大于16.
	int rank = 0;//16bits表示的几个完整的runs,假设第一个runs是1-runs,这几个runs的rank值。
	int runs = 0 ;//runs 个数.
	
	int x = 0;//工作变量，保存本次的gamma解码值.
	int prestep = 0;//前一次正确解码的bits数(累加),<=16.
	for(u64 i=0;i<tablesize;i++)
	{
		B[0] = (i<<48);
		sum  =0 ;
		step = 0;
		prestep=0;
		rank = 0;
		runs = 0;
		while(1)
		{
			x = GammaDecode(B,step,this);//step会联动.
			if(step > 16)
				break;
			sum = sum + x;
			prestep = step;
			runs ++;
			if(runs%2==1)
				rank = rank + x;
		}
		R[i<<2] = runs;//r4
		R[(i<<2)+1] = prestep;//r2
		R[(i<<2)+2] = sum; //r1;
		R[(i<<2)+3] = rank;//r3

	}
}

//递归保存节点的编号信息
int ABS_FM::SaveNodePosition(BitMap * r,u32 position,savekit &s)
{
	if(!r)
		return 1;
	s.writei32(position);
	SaveNodePosition(r->Left(), 2 * position,s);
	SaveNodePosition(r->Right(),2 * position +1,s);
	return 0;
}

//递归保存节点的数据信息
int ABS_FM::SaveNodeData(BitMap *r,savekit &s)
{
	if(!r)
		return 1 ;
	r->Save(s);
	SaveNodeData(r->Left(),s);
	SaveNodeData(r->Right(),s);
	return 0;
}

int ABS_FM::SaveWTTree(savekit &s)
{
	//保存编号信息
	//int nodecount = 2*alphabetsize -1;
	//s.writei32(nodecount);
	SaveNodePosition(root,1,s);

	//保存节点数据信息
	SaveNodeData(root,s);
	return 0;
}

int ABS_FM::LoadWTTree(loadkit &s,uchar **tables)
{
	//读取数据，map的int域对应该节点的位置
	int nodecount = 2*alphabetsize -1;
//	cout<<alphabetsize<<endl;
//	s.loadi32(nodecount);
	int * p = new int[nodecount];
	s.loadi32array(p,nodecount);
	map<int,BitMap * > pmap;
	BitMap * r=NULL;
	for(int i=0;i<nodecount;i++)
	{
		if(tables)
			r = new BitMap(tables);
		else
			r = new BitMap();
		r->Load(s);
		pmap[p[i]] = r;
	}
	//挂链
	map<int ,BitMap *>::iterator iter;
	map<int ,BitMap *>::iterator f_iter;
	for(iter = pmap.begin();iter!=pmap.end();iter++)
	{
		f_iter = pmap.find(2*iter->first);
		if(f_iter != pmap.end())
			iter->second->Left(f_iter->second);
		else
			iter->second->Left(NULL);
		
		f_iter = pmap.find(2*iter->first +1);
		if(f_iter!=pmap.end())
			iter->second->Right(f_iter->second);
		else
			iter->second->Right(NULL);
	}
//	cout<<"767"<<endl;	
	f_iter = pmap.find(1);
	if(f_iter !=pmap.end())
		this->root = f_iter->second;
	else
	{
		cerr<<"Load WTTree error"<<endl;
		this->root = NULL;
		exit(0);
	}
//	cout<<"778"<<endl;
	return 0;
}

int ABS_FM::Load(loadkit &s)
{
	s.loadi32(this->n);
	s.loadi32(this->alphabetsize);
	s.loadi32(this->D);

	//for C
	this->C = new int[alphabetsize+1];
	s.loadi32array(this->C,alphabetsize+1);
	//for code
	this->code = new int[256];
	s.loadi32array(this->code, 256);  

	//for codeTable;
	memset(codeTable,0,sizeof(codeTable));
	for(int i=0;i<256;i++)
	{
		uchar len=0;
		s.loadu8(len);
		if(len!=0)
		{
			int bytes = len%8?len/8+1:len/8;
			uchar * bits = new uchar[bytes];
			s.loadu8array(bits,bytes);
			int in_index =0;
			int off_index =0;
			for(int j=0;j<len;j++)
			{
				if(bits[off_index] & (0x01<<(7-in_index)))
					codeTable[i][j] = '1';
				else
					codeTable[i][j] = '0';
				in_index++;
				if(in_index==8)
				{
					in_index =0;
					off_index ++;
				}
			}
		}
	}
	
	//for SAL
	this->SAL = new InArray();
	this->SAL->load(s);
	//for Rankl
	this->RankL = new InArray();
	this->RankL->load(s);

	Inittable();
	uchar * par[2]={Z,R};
	//cout<<"cs"<<endl;
	LoadWTTree(s,par);
//	cout<<"835"<<endl;
	T=NULL;
	bwt=NULL;
	return 0;
}
int ABS_FM::Save(savekit &s)
{
	s.writei32(n);
	s.writei32(alphabetsize);
	s.writei32(D);//SA的采样率
	
	//C表
	//s.writei32(alphabetsize+1);
	s.writei32array(C,alphabetsize+1);
	//code表
	//s.writei32(256);
	// s.writeu8array(code,256);
	s.writei32array(code, 256);  //kkzone-bai debug
	
	//codeTable
	for(int i=0;i<256;i++)
	{
		uchar len = strlen(codeTable[i]);
		s.writeu8(len);
		if(0!=len)
		{
			int bytes = len%8?len/8+1:len/8;
			uchar *bits = new uchar[bytes];
			memset(bits,0,bytes);
			int off_index=0;
			int in_index =0;
			for(int j=0;j<len;j++)
			{
				if(codeTable[i][j]=='1')
					bits[off_index] = bits[off_index]|(0x01<<(7-in_index));
				in_index++;
				if(8==in_index)
				{
					in_index=0;
					off_index++;
				}
			}
			s.writeu8array(bits,bytes);
		}
	}
	
	//for SAL
	SAL->write(s);
	//for RankL
	RankL->write(s);
	//for WT tree
//	cout<<"SaveWTTree"<<endl;
	SaveWTTree(s);
	return 0;
}
void ABS_FM::deCompress()
{
}
