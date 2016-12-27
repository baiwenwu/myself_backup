#ifndef _WZIP_H
#define _WZIP_H
#define _FILE_OFFSET_BITS 64
#include<iostream>
#include<fstream>
using namespace std;
//#include <stdlib.h>
//#include <stdio.h>

#define  CHAR_SET_SIZE		256
#define  CODE_MAX_LEN		256

//all kinds of error code
#define  ERR_PARASE_ARG		-1
#define  ERR_PARAMETER		-2
#define  ERR_IO				-3
#define  ERR_MEMORY			-4
#define  ERR_CRC_CHECK		-5
#define  ERR_FILE_NAME		-6


//compressed file's head/tail const character
#define  WZIP_W				'w'
#define  WZIP_Z				'z'
#define  WZIP_I				'i'
#define  WZIP_P				'p'
#define  WZIP__				'_'
#define  WZIP_H				'h'
#define  WZIP_T				't'

//block boundary marker
#define  BLK_HEAD_MARK		0xffffffff
#define  BLK_TAIL_MARK		0x00000000

#define  FILE_NAME_LEN		1024

#define  FILE_HEAD_LEN		6
#define  FILE_TAIL_LEN		6

#define OVERSHOOT 1000

#define  HUN_K 100000
typedef unsigned char	uchar;
typedef unsigned int	u32;
typedef unsigned short u16;
typedef unsigned short	ushort;

typedef void(*sigHandler)(int);

typedef enum Mode{
	COMPRESS,
	DECPRESS
};


typedef enum TreeType{
	HUFFMAN,
	BALANCE,
	HU_TACKER
};

typedef enum NodeCodeType{
	RLE_GAMA,
	RLE_DELTA,
	HBRID
};

//for huffman code

typedef struct huffNode_t{
	u32	freq;
	uchar label;
	char code[CODE_MAX_LEN];
	struct huffNode_t *leftChild;
	struct huffNode_t *righChild;
}huffNode_t;

typedef huffNode_t * huffmanTree;

//for balance code
typedef struct balNode_t{
	uchar set[CHAR_SET_SIZE];
	u32 setSiz;
	uchar label;
	struct balNode_t *leftChild;
	struct balNode_t *righChild;
	char code[CODE_MAX_LEN];
}balNode_t;

typedef balNode_t *balanceTree;
//for hu-tacker code
typedef struct hutaNode_t{
	u32 freq;
	uchar label;
	int level;
	struct hutaNode_t *leftChild;
	struct hutaNode_t *righChild;
}hutaNode_t;
typedef hutaNode_t * hutackerTree;
#endif