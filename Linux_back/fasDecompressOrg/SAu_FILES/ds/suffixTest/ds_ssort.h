/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
   Prototypes for the Deep Shallow Suffix Sort routines
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */ 
#ifndef _DS_SSORT_H
#define _DS_SSORT_H

#ifdef __cplusplus
extern "C"
{
void ds_ssort(unsigned char *t, unsigned long *sa, long n);
int init_ds_ssort(int adist, int bs_ratio);
}
#endif

void ds_ssort(unsigned char *t,unsigned long *sa,long n);
int init_ds_ssort(int adlist,int bs_ratio);

#endif 
