#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "ds_ssort.h"
extern "C"{
#include "ds_ssort.h"
}

int main(int argc,char *argv[]){

    unsigned char buff[256]="abraca";
	//memset(buff,0,sizeof(buff));
    int n,overshoot,i;
    unsigned char *text;
    //unsigned long *sa;
    unsigned int*sa;
    overshoot=init_ds_ssort(500,2000);
    printf("overshoot=%d\n",overshoot);
	if(overshoot==0){
        printf("init_ds_ssort error\n");
        exit(0);
    }
    
    n=strlen((char*)buff);
    //n=10;
	printf("n=%d\n",n);
    sa=(unsigned int*)malloc(n*sizeof(unsigned));
    if(!sa){
        printf("malloc error\n");
        exit(0);
    }
    
    text=(unsigned char*)malloc((n+overshoot)*sizeof(unsigned char));
    if(!text){
        printf("malloc error\n");
        exit(0);
    }
    
    memcpy(text,buff,n);
    ds_ssort(text,(unsigned long *)sa,n);
    

    for(i=0;i<n;i++){
        printf("%ld ",sa[i]);
    }
    printf("\n");
    exit(0);
}
