#ifndef FM_H
#define FM_H
#include"loadkit.h"
#include"savekit.h"
#include"ABS_WT.h"
#include"Huffman_WT.h"
#include"Balance_WT.h"
#include"Hutacker_WT.h"
#include"WT_Handle.h"
class FM
{
	public:
		FM(const char * filename,int speedlevel=1);
		FM();
		~FM(){};
		FM(const FM & h):wt(h.wt){}
		FM& operator =(const FM&h){wt=h.wt;return *this;};
		int load(const char * indexfile);
		int save(const char * indexfile);

		int getN();
		int getAlphabetSize();
		int sizeInByte();
		double compressRatio();
		//kkzone 2016-09-06
		void deCompress();
		
	private:
		WT_Handle wt;
};
#endif

