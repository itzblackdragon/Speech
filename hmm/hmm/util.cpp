#include"util.h"
#include"log.h"
#include<math.h>
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#define pi	22.0 / 7
/*
	@param file_ptr is the file pointer
	@param file_name is the name of the file to be opened.
	@param mode is the mode of operation on the file
	@return file_ptr is the modified file pointer now pointed to the input file
*/
FILE * file_open(FILE * file_ptr,const char * file_name,const char * mode){
	errno_t err;
	err = fopen_s(&file_ptr,file_name,mode);
	if (err){
			printf("%s creation failed\n", file_name);
			fclose(file_ptr);
			exit(0);
	}
	return file_ptr;
}
/*
	@param file_ptr is the file pointer currently pointing to some file
*/
extern void file_close(FILE * file_ptr){
	fclose(file_ptr);
}
/*
	@param E is the error
	@param K is the parker coefficients
	@param R is the auto correlation vector
	@param alpha is the LPC coefficients
*/
void durbins_algorithm(float * E, float * K, float * R, float * alpha,const unsigned int order ){
	unsigned int i , j;
	float * alpha_temp = (float *)malloc(sizeof(float) * (order + 1));
	float temp;
	E[0] = R[0]; //log_write("E[0] = %.3f\n", E[0]);
	for(i = 1; i < order + 1; i++){
		temp = 0;
		for(j = 1; j < i; j++)
			 temp += alpha[j] * R[i - j];
		K[i] = (R[i] - temp) / E[i - 1];	//log_write("K[%d] = %.3f\n", i, K[i]);
		for(j = 1; j < i; j++)
			alpha_temp[j] = alpha[j];
		for (j = 1; j < i; j++){
			alpha[j] = alpha_temp[j] - (K[i] * alpha_temp[i - j]);
			//log_write("a[%d] = %.3f  ", j, alpha[j]);
		}
		alpha[i] = K[i];	//log_write("A[%d] = %.3f\n", i, alpha[i]);
		E[i] = (1 - SQUARE(K[i])) * E[i - 1]; //log_write("E[%d] = %.3f\n", i,E[i]);
	}
	for(i = 0; i < order ; i++){
		alpha[i] = alpha[i + 1];
		//log_write("%f\t", alpha[i]);	// even though indexing start from 0 we are displaying it from 1
	}
	//log_write("\n");
	free(alpha_temp);
}
/*
	@param value is the number whose sign is to be determined
	@return 1 if value is +ve 0 otherwise
*/
int sign(float value){
	if (value >= 0){
		return 1;
	}
	else{
		return -1;
	}
}
/*
	@param vlaue is the value to be mapped to new range
	@param source_min is the minimum value of the source range
	@param source_max is the maximum value of the source range
	@param dest_min is the minimum value of the destination range
	@param dest_max is the maximum value of the destination range
	@return new value in the destination range
*/
float map_to_range(float value, float source_min, float source_max, float dest_min, float dest_max){
	return (((value - source_min) / (source_max - source_min)) * (dest_max - dest_min)) + dest_min;
}
/*
	@param ref is the reference vector
	@param test is the test vector
	@param size is the size of input vectors, size <= 12
	@return is the tokhura distance between the vectors D = (1 / size) SIGMA ( (ref(i) - test(i)) * w(i))
*/
float tokhura_distance(float ref[], float test[],int size){
	int Tokhura_weight[] = { 1 , 3, 7, 13, 19, 22, 25, 32, 42, 50, 56, 61};
	int i;
	float d = 0;
	for(i = 0; i < size; i++){
		d += Tokhura_weight[i] * (ref[i] - test[i]) * (ref[i] - test[i]);
	}
	return d / size;
}
/*
	@param n is the positio of the data from the start
	@param N is the number of data points in the window
	@return 0.54 - 0.46 cos( 2 pi n / N - 1) if n < N, 0 otherwise
*/
float compute_h(int n, int N){
	if (n >= 0 && n < N)
		return (float) (0.54 - 0.46 * cos((float)(2 * pi * n) / N - 1));
	else return 0;
}
/*
	@param m is the index of cepstral coefficient
	@param Q is the order of cepstral coefficients
	@return is 1 + Q / 2 sin(pi * m / Q), 0 < m <= Q
*/
float cep_weight(int m, int Q){	// FIXME: first value is very high
	if(m > 0 && m <= Q)
		return (float)(1 + (Q / 2) * sin(pi * m / Q));
	else return 0;
}

long double ** create_matrix(long double ** matrix, int row, int column){
	int i;
	matrix = (long double **) malloc(sizeof(long double *) * row);
	for(i = 0; i < row; i++)
		matrix[i] = (long double *) malloc(sizeof(long double) * column);
	return matrix;
}
void dispose_matrix(long double ** matrix, int row, int column){
	int i;
	assert(matrix != NULL);
	for(i = 0; i < row; i++)
		if(matrix[i]) free(matrix[i]);
	free(matrix);
}
int * get_observation(int * obs,const char * file_name, int * obs_len){
	int i;
	FILE * data = NULL;
	int length;
	data = file_open(data, file_name, "r");
	fscanf(data, "%d", &length);
	obs = (int *)malloc(sizeof(int) * length);
	for(i = 0; i < length; i++)
		fscanf(data,"%d",&obs[i]);
	* obs_len = length;
	file_close(data);
	return obs;
}
