#ifndef	__LAMDA_H__
#define	__LAMDA_H__

typedef struct{
	int __M__;
	int __N__;
	unsigned long double * __PI__;
	unsigned long double  ** __A__;
	unsigned long double  ** __B__;
}LAMDA;

extern LAMDA * lamda_create(LAMDA * l, int num_states, int num_symbols);
extern void lamda_dispose(LAMDA * l);
//TODO:
extern LAMDA * lamda_load(LAMDA * l, const char * file_name);
extern LAMDA * lamda_reload(LAMDA * l, const char * file_name);
extern void lamda_store(LAMDA * l, const char * file_name);
extern LAMDA * lamda_merge(LAMDA * l1, LAMDA * l2);
extern LAMDA * lamda_correction(LAMDA * l);
#endif