#ifndef __CODEBOOK_H__
#define __CODEBOOK_H__

typedef struct{
	float ** __CODEBOOK__;
	int __ORDER__;
	int __SIZE__;
}CODEBOOK;
extern CODEBOOK * codebook_create(CODEBOOK * c,int size, int order);
extern void codebook_dispose(CODEBOOK * c);
extern CODEBOOK * coadbook_load(CODEBOOK * c, const char * file_name);
extern void codebook_store(CODEBOOK * c, const char * file_name);

#endif