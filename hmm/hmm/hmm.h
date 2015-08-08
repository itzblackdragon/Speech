#ifndef __HMM_H__
#define __HMM_H__
#include"lamda.h"
typedef struct{
	LAMDA * __L__;
	long double ** __ALPHA__;
	long double ** __BETA__;
	long double ** __GAMMA__;
	long double *** __ZETA__;
	long double __PROB__;
	long double __P_STAR__;
	int * __Q_STAR__;
	int __T__;
}HMM;
//extern HMM * hmm_load(HMM * h,const char * file_name);
extern void hmm_train(const char * obs_file, const char * hmm_file);
extern void hmm_test(const char * file_name);
extern HMM * forward_procedure(HMM * h, int * obs, int obs_len);
extern void hmm_dispose(HMM * h);
extern void hmm_merge(const char * hmm_file1, const char * hmm_file2);
#define	HMM_TRAIN(hmm_file)	\
	hmm_train(DEFAULT_OBS, hmm_file)
#endif