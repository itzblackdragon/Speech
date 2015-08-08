#include"lamda.h"
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"util.h"
#include"log.h"
LAMDA * lamda_create(LAMDA * l, int num_states, int num_symbols){
	l = (LAMDA *)malloc(sizeof(LAMDA));
	assert(l != NULL);
	l -> __PI__ = (long double *) malloc(sizeof(long double) * num_states);
	l -> __A__ = create_matrix(l -> __A__, num_states, num_states);
	l -> __B__ = create_matrix(l -> __B__, num_states, num_symbols);
	l -> __N__ = num_states;
	l -> __M__ = num_symbols;
	return l;
}

void lamda_dispose(LAMDA * l){
	if(l){
		free(l -> __PI__);
		dispose_matrix(l -> __A__,l -> __N__, l -> __N__);
		dispose_matrix(l -> __B__, l -> __N__, l -> __M__);
		free(l);
	}
}
/*
	@param l is the model to be stored
	@param file_name is the name of the file to which l is to be stored
*/
void lamda_store(LAMDA * l, const char * file_name){
	int i, j;
	int M = l -> __M__;
	int N = l -> __N__;
	FILE * data = NULL;
	data = file_open(data, file_name, "w");
	//fprintf(data,"#states\n");
	fprintf(data, "%d\n", l -> __N__);
	//fprintf(data, "#symbols\n");
	fprintf(data, "%d\n", l -> __M__);
	for(i = 0; i < N; i++)
		fprintf(data, "%lg ", l -> __PI__[i]);
	fprintf(data,"\n");
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++)
			fprintf(data, "%lg ", l -> __A__[i][j]);
		fprintf(data,"\n");
	}
	for(i = 0; i < N; i++){
		for(j = 0; j < M; j++)
			fprintf(data, "%lg ", l -> __B__[i][j]);
		fprintf(data,"\n");
	}
	file_close(data);
}
/*
	@param l is the model to be loded 
	@param file_name is the name of the file from which l is to be loaded
*/
LAMDA * lamda_load(LAMDA * l, const char * file_name){
	FILE * data = NULL;
	int i = 0, j = 0;
	int num_states;
	int num_symbols;
	data = file_open(data, file_name,"r");
	fscanf(data, "%d", &num_states);
	fscanf(data, "%d", &num_symbols);
	l = lamda_create(l, num_states, num_symbols);	// initialize lamda
	for(i = 0; i < num_states; i++)
		fscanf(data, "%lg", &(l -> __PI__[i]));
	for(i = 0; i < num_states; i++)
		for(j = 0; j < num_states; j++)
			fscanf(data, "%lg", &(l -> __A__[i][j]));
	for(i = 0; i < num_states; i++)
		for(j = 0; j < num_symbols; j++)
			fscanf(data, "%lg", &(l -> __B__[i][j]));
	file_close(data);
	return l;
}
LAMDA * lamda_reload(LAMDA * l, const char * file_name){
	if(l)	lamda_dispose(l);
	l = lamda_load(l, file_name);
	return l;
}
LAMDA * lamda_merge(LAMDA * l1, LAMDA * l2){
	int i, j;
	int M = l1 -> __M__;
	int N = l1 -> __N__;
	//assert((l1 -> __N__ != l2 ->__N__) || (l1 -> __M__ != l2 -> __M__));
	for(i = 0; i < N; i++)
		l1 -> __PI__[i] = (l1 -> __PI__[i] + l2 -> __PI__[i]) / 2;	
	for(i = 0; i < N; i++)
		for(j = 0; j < N; j++)
			l1 -> __A__[i][j] = (l1 -> __A__[i][j] + l2 -> __A__[i][j]) / 2;
	for(i = 0; i < N; i++)
		for(j = 0; j < M; j++)
			l1 -> __B__[i][j] = (l1 -> __B__[i][j] + l2 -> __B__[i][j]) / 2;
	return l1;
}
LAMDA * lamda_correction(LAMDA * l){
	int i, j;
	int M = l -> __M__;
	int N = l -> __N__;
	long double total_correction = 0.01;
	long double correction = 0;
	long double ** B = l -> __B__;
	int * zero_cell = (int *)malloc(sizeof(int) * M);
	int zero_cell_count = 0;
	long double max = 0;
	int max_index = 0;
	for(i = 0; i < N; i++){
		zero_cell_count = 0;
		for(j = 0; j < M; j++){
			if(B[i][j] > 0){
				zero_cell[j] = 0;
				if(B[i][j] > max){
					max = B[i][j];
					max_index = j;
					//log_write("max = %lg at %d\n", B[i][j], j);
				}
			}
			else{
				if(B[i][j] < 0)	B[i][j] = 0;
				zero_cell[j] = 1;
				//log_write("zero cell : %d\n", j);
				zero_cell_count++;
			}
		}
		//log_write("cell count : %d\n", zero_cell_count);
		if(zero_cell_count)	correction = total_correction / zero_cell_count;
		for(j = 0; j < M; j++){
			if(zero_cell[j] == 1)	B[i][j] = correction;
			else if(j == max_index)	B[i][j] -= total_correction;
		}
	}
	free(zero_cell);
	return l;
}