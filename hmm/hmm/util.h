#ifndef	__UTIL_H__
#define	__UTIL_H__
#include<stdio.h>

#define	MAX(a,b)	(a) > (b) ? (a) : (b)
#define MIN(a,b)	(a) < (b) ? (a) : (b)
#define	SQUARE(a)	(a)	* (a)

typedef enum{
	FALSE,
	TRUE
}Bool;

extern int sign(float i);
extern FILE * file_open(FILE * file_ptr,const char * file_name,const char * mode);
extern void file_close(FILE * file_ptr);
extern float map_to_range(float value, float source_min, float source_max, float dest_min, float dest_max);
extern void durbins_algorithm(float * E, float * K, float * R, float * alpha,const unsigned int order );
extern float tokhura_distance(float ref[], float test[],int size);
extern float compute_h(int n, int x);
extern float cep_weight(int m, int Q);
extern long double ** create_matrix(long double ** matrix, int row, int column);
extern void dispose_matrix(long double ** matrix, int row, int column);
extern int * get_observation(int * obs,const char * file_name, int * obs_len);
#endif