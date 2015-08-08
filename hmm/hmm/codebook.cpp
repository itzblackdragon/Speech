#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"log.h"
#include"codebook.h"
#include"util.h"
extern CODEBOOK * codebook_create(CODEBOOK * c, int size, int order){
	int i;
	c = (CODEBOOK *) malloc(sizeof(CODEBOOK));
	assert(c != NULL);
	c -> __CODEBOOK__ = (float **)malloc(sizeof(float *) * size);
	for(i = 0; i < size; i++)
		c -> __CODEBOOK__[i] = (float *)malloc(sizeof(float) * order);
	c -> __SIZE__ = size;
	c -> __ORDER__ = order;
	return c;
}
extern void codebook_dispose(CODEBOOK * c){
	int i;
	if(c){
		for(i = 0; i < c -> __SIZE__; i++)
			free(c -> __CODEBOOK__[i]);
		free(c);
	}
}
extern CODEBOOK * coadbook_load(CODEBOOK * c, const char * file_name){
	FILE * data = NULL;
	int order;
	int size;
	int i, j;
	data = file_open(data, file_name, "r");
	fscanf(data, "%d", &size);
	fscanf(data, "%d", &order);
	c = codebook_create(c, size, order);
	for(i = 0; i < size;i++)
		for(j = 0; j < order; j++)
			fscanf(data, "%f", &(c -> __CODEBOOK__[i][j]));
	return c;
}
extern void codebook_store(CODEBOOK * c, const char * file_name){
	FILE * data = NULL;
	int i, j;
	int size = c -> __SIZE__;
	int order = c -> __ORDER__;
	data = file_open(data, file_name, "w");
	fprintf(data, "%d\n", size);
	fprintf(data, "%d\n", order);
	for(i = 0; i < size ;i++){
		for(j = 0; j < order; j++)
			fprintf(data, "%f ", c -> __CODEBOOK__[i][j]);
		fprintf(data, "\n");
	}
	file_close(data);
}