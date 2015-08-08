#include"lamda.h"
#include"hmm.h"
#include"log.h"
#include"util.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<string.h>
#define	BUFFER_SIZE	1024
#define __DEFAULT_HMM__	"temp/hmm/default/default.hmm"
#define HMM_CONF	"hmm.config"

/*
	@param h is the hmm to be constructed
	@param O is the observation sequence
	@param T is the length of the observation sequence
	@return h is the updated hmm(with alpha values) 
*/
HMM * forward_procedure(HMM * h, int * O, int T){	// compute the probability of observation given the model
	//log_write("\n================================================ alpha matrix =============================================\n");
	int i, j;
	long double temp = 0;
	int t;
	long double ** alpha = h ->__ALPHA__;
	long double * pi = h -> __L__ -> __PI__;
	long double ** A = h -> __L__ -> __A__;
	long double ** B = h -> __L__ -> __B__;
	int M = h -> __L__ -> __M__;
	int N = h ->__L__ -> __N__;
	//initialisation
	for(i = 0; i < N; i++){
		alpha[0][i] = pi[i] * B[i][O[0]];
		//log_write("%llg\t", alpha[0][i]);
	}
	//log_write("\n");
	// induction
	for(t = 0; t < T - 1; t++){
		for(j = 0; j < N; j++){
			temp = 0;
			for(i = 0; i < N; i++)
				temp += alpha[t][i] * A[i][j];
			alpha[t + 1][j] = temp * B[j][O[t + 1]];
			if(alpha[t + 1][j] < 0)	alpha[t + 1][j] = 0;
		}
	}
	/*for(t = 0; t < T; t++){
		for(j = 0; j < N; j++)
			log_write("%llg\t", alpha[t][j]);
		log_write("\n");
	}*/
	temp = 0;
	for(j = 0; j < N; j++)
		temp += alpha[T - 1][j];
	//log_write("probability : %llg\n", temp);
	h -> __PROB__ = temp;
	//log_write("\n===================================================================================================\n");
	return h;
}
/*
	@param h is the hmm to be constructed
	@param O is the observation sequence
	@param T is the length of the observation sequence
	@return h is the updated hmm(with beta values) 
*/
HMM * backward_procedure(HMM * h, int * O, int T){
	long double ** beta = h ->__BETA__;
	long double * pi = h -> __L__ -> __PI__;
	long double ** A = h -> __L__ -> __A__;
	long double ** B = h -> __L__ -> __B__;
	int M = h -> __L__ -> __M__;
	int N = h -> __L__ -> __N__;
	//log_write("\n================================================ Beta matrix =============================================\n");
	int i, j, t;
	long double temp;
	for(i = 0; i < N; i++)
		beta[T - 1][i] = 1;
	// induction
	for(t = T - 2; t >= 0; t--){
		for(i = 0; i < N; i++){
			temp = 0;
			for(j = 0; j < N; j++){
				temp += A[i][j] * B[j][O[t + 1]] * beta[t + 1][j];
				//log_write("A[%d][%d]:%llg\tB[%d][%d]:%llg\n", i, j, A[i][j], j, t + 1, B[j][O[t + 1]]);
			}
			beta[t][i] = temp ;	//log_write("beta[%d][%d]:%llg\n", t, i, beta[t][i]);
			if(beta[t][i] < 0)	beta[t][i] = 0;	// check for overflow
		}
	}
	/*for(t = 0; t < T; t++){
		for(i = 0; i < N; i++)
			log_write("%llg\t", beta[t][i]);
		log_write("\n");
	}*/
	//log_write("\n====================================================================================\n");
	return h;
}
/*
	@param h is the hmm to be constructed
	@param O is the observation sequence
	@param T is the length of the observation sequence
	@return h is the updated hmm(with p_star & q_star values) 
*/
HMM *  viterbi(HMM * h, int * O, int T){
	long double * pi = h -> __L__ -> __PI__;
	long double ** A = h -> __L__ -> __A__;
	long double ** B = h -> __L__ -> __B__;
	int M = h -> __L__ -> __M__;
	int N = h -> __L__ -> __N__;
	int i, j, t;
	long double ** delta = NULL;
	int ** xi = NULL; 
	long double max = 0;
	int arg_max = 0;
	long double temp = 0;
	long double p_star = 0;
	int * q_star = h -> __Q_STAR__;
	
	delta = create_matrix(delta, T, N);
	xi = (int **) malloc(sizeof(int *) * T);
	for(t = 0; t < T; t++)
		xi[t] = (int *) malloc(sizeof(int) * N);
	
	//initialization
	for(i = 0; i < N; i++){
		delta[0][i] = pi[i] * B[i][O[0]];
		xi[0][i] = 0;
		//log_write("delta[0][%d]:%llg\n", i, delta[0][i]);
	}
	
	//rescursion
	for(t = 1; t < T; t++){
		for(j = 0; j < N; j++){
			max = 0;
			for(i = 0; i < N; i++){
				temp = delta[t - 1][i] * A[i][j];
				max = MAX(max, temp);
				if(max == temp)	arg_max = i;
			}
			delta[t][j] = max * B[j][O[t]];	//log_write("%llg ", temp);;
			if(delta[t][j] < 0)	delta[t][j] = 0;	// check for over flow
			xi[t][j] = arg_max;	
			//log_write("\ndelata[%d][%d]: %llg\tsi[%d][%d]:%d\n",t, j, max, t, j, xi[t][j]);
		}
	}
	max = 0;
	for(i = 0; i < N; i++){
		max = MAX(max , delta[T - 1][i]);
		if(max == delta[T - 1][i])	arg_max = i;
	}
	p_star = max;
	//log_write("P *= %llg\n", p_star);
	q_star[T - 1] = arg_max;
	for(t = T - 2; t >= 0; t--){
		q_star[t] = xi[t + 1][q_star[t + 1]];
		//log_write("%d ",q_star[t]);
	}
	/*log_write("State: ");
	for(t = 0; t < T; t++)
		log_write("%d ",q_star[t]);
	log_write("\n");*/
	h -> __P_STAR__ = p_star;
	dispose_matrix(delta, T, N);
	for(t = 0; t < T; t++)
		free(xi[t]);
	free(xi);
	return h;
}
/*
	@param h is the hmm to be constructed
	@param T is the length of the observation sequence
	@return h is the updated hmm(with gamma values) 
*/
HMM * compute_gamma(HMM * h, int T){
	long double ** alpha = h ->__ALPHA__;
	long double ** beta = h ->__BETA__;
	long double ** gamma = h -> __GAMMA__;
	int N = h -> __L__ -> __N__;
	//log_write("\n================================================ gamma matrix =============================================\n");
	int i, t;
	long double temp;
	for(t = 0; t < T; t++){
		temp = 0;
		for(i = 0; i < N; i++)
			temp += alpha[t][i] * beta[t][i];
		for(i = 0; i < N; i++){
			gamma[t][i] = alpha[t][i] * beta[t][i] / temp;
			//log_write("%llg\t", gamma[t][i]);
			if(gamma[t][i] < 0)	gamma[t][i] = 0;	//check for overflow
		}
		//log_write("\n");
	}
	//log_write("\n==================================================================================================\n");
	return h;
}
/*
	@param h is the hmm to be constructed
	@param O is the observation sequence
	@param T is the length of the observation sequence
	@return h is the updated hmm(with zeta values) 
*/
HMM * compute_zeta(HMM * h, int * O, int T){
	long double ** A = h -> __L__ -> __A__;
	long double ** B = h -> __L__ -> __B__;
	long double ** alpha = h ->__ALPHA__;
	long double ** beta = h ->__BETA__;
	long double *** zeta = h -> __ZETA__;
	int N = h -> __L__ -> __N__;
	//log_write("\n============================================= Zeta matrix ======================================\n");
	int t, i, j;
	long double temp = 0;
	for(t = 0; t < T - 1; t++){
		temp = 0;
		for(i = 0; i < N; i++)
			for(j = 0; j < N; j++)
				temp += alpha[t][i] * A[i][j] * B[j][O[t + 1]] * beta[t + 1][j];
		for(i = 0; i < N; i++){
			for(j = 0; j < N; j++){
				zeta[t][i][j] = alpha[t][i] * A[i][j] * B[j][O[t + 1]] * beta[t + 1][j] / temp; 
				//log_write("%llg\t", zeta[t][i][j]);
				if(zeta[t][i][j] < 0)	zeta[t][i][j] = 0;	// check for overflow
			}
			//log_write("\n");
		}
		//log_write("\n");
	}
	//log_write("\n==================================================================================================\n");
	return h;
}
/*
	@param h is the hmm to be updated
	@return h is the updated hmm
*/
HMM * update_pi(HMM * h){
	long double * pi = h -> __L__ -> __PI__;
	long double ** gamma = h -> __GAMMA__;
	int N = h -> __L__ -> __N__;
	//log_write("\n============================================= PI vector ======================================\n");
	int i;
	for(i = 0; i < N; i++){
		pi[i] = gamma[0][i];	
		//log_write("%llg\t", gamma[0][i]);
	}
	//log_write("\n");
	//log_write("\n==================================================================================================\n");
	return h;
}
/*
	@param h is the hmm to be updated
	@return h is the hmm whose A matrix is updated
*/
HMM * update_A(HMM * h, int T){
	long double ** A = h -> __L__ -> __A__;
	long double ** gamma = h -> __GAMMA__;
	long double *** zeta = h -> __ZETA__;
	int N = h -> __L__ -> __N__;
	//log_write("\n============================================= A Matrix ======================================\n");
	int i, j, t;
	long double num = 0, denom = 0;
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			num = 0;
			denom = 0;
			for(t = 0; t < T - 1; t++){
				denom += gamma[t][i];
				num += zeta[t][i][j];
			}
			A[i][j] = num / denom;	//log_write("%llg\t", A[i][j]);
		}
		//log_write("\n");
	}
	//log_write("\n==================================================================================================\n");
	return h;
}
/*
	@param h is the hmm to be constructed
	@param O is the observation sequence
	@param T is the length of the observation sequence
	@return h is the updated hmm(with B values) 
*/
HMM * update_B(HMM * h, int * O, int T){
	long double ** B = h -> __L__ -> __B__;
	long double ** gamma = h -> __GAMMA__;
	int M = h -> __L__ -> __M__;
	int N = h -> __L__ -> __N__;
	//log_write("\n============================================= B matrix ======================================\n");
	int j, k, t;
	long double num = 0;
	long double denom = 0;
	for(j = 0; j < N; j++){
		for(k = 0; k < M; k++){
			num = 0;
			denom = 0;
			for(t = 0; t < T; t++){
				if(O[t] == k)	num += gamma[t][j];
				denom += gamma[t][j];	
			}
			B[j][k] = num / denom;	//log_write("%llg\t", B[j][k]);
		}
		//log_write("\n");
	}
	//log_write("\n==================================================================================================\n");
	return h;
}
/*
	@param h is the hmm to be updated
	@param O is the observation sequence
	@param T is the length of the observation sequence
	@return h is the updated hmm 
*/
HMM * update_model(HMM * h, int * O, int T){
	h = forward_procedure(h,O,T);	// alpha
	h = backward_procedure(h, O, T); // beta
	h = compute_gamma(h, T);	// gamma
	h = compute_zeta(h, O, T);
	if(h -> __PROB__ > 0){
		h = update_pi(h);
		h = update_A(h, T);
		h = update_B(h, O, T);
	}
	return h;
}
/*
	@param h is the hmm to be initialized
	@param obs_len is the length of the observation(T)
	@return h is the initialized hmm(Lamda is initialized seperatly using lamda_load() separatly)
*/
HMM * hmm_init(HMM * h, int obs_len){
	int t;
	//if(!h)	h = h = (HMM *) malloc(sizeof(HMM));
	h ->__T__ = obs_len;
	h -> __ALPHA__ = create_matrix(h -> __ALPHA__, obs_len, h -> __L__ ->__N__);
	h -> __BETA__ = create_matrix(h -> __BETA__, obs_len, h -> __L__ ->__N__);
	h -> __GAMMA__ = create_matrix(h -> __GAMMA__, obs_len, h -> __L__ ->__N__);
	h -> __ZETA__ = (long double ***)malloc(sizeof(long double **) * obs_len);
	for(t = 0; t < obs_len; t++)
		h -> __ZETA__[t] = create_matrix(h ->__ZETA__[t], h -> __L__ ->__N__, h -> __L__ ->__N__);
	h -> __Q_STAR__ = (int *) malloc(sizeof(int) * obs_len);
	return h;
}
/*
	@param h is the hmm to be constructed
	@param file_name is the name of the file from which hmm is to be loaded
	@return h is the hmm loaded from file_name
*/
HMM * hmm_load(HMM * h,const char * file_name){
	h = (HMM *) malloc(sizeof(HMM));
	assert(h != NULL);
	h -> __L__ = lamda_load(h -> __L__, file_name);
	return h;
}
/*
	@param h is the hmm to be reloaded
	@param file_name is the name of the file from which hmm is to be reloaded
	@return h is the hmm loaded from file_name(here only lamda is upadating)
*/
HMM * hmm_reload(HMM * h, const char * file_name){
	assert(h != NULL);
	h -> __L__ = lamda_reload(h -> __L__, file_name);
	return h;
}
/*
	@param h is the hmm to be constructed
	@param file_name is the name of the file to which hmm is to be stord
*/
void hmm_store(HMM * h, const char * file_name){
	assert(h != NULL);
	lamda_store(h -> __L__, file_name);
}
/*
	@param h is the hmm to be displayed
*/
void hmm_print(HMM * h){
	int i, j, t;
	long double * pi = h -> __L__ -> __PI__;
	long double ** A = h -> __L__ -> __A__;
	long double ** B = h -> __L__ -> __B__;
	int M = h -> __L__ -> __M__;
	int N = h -> __L__ -> __N__;
	int T = h -> __T__;
	log_write("===================================================================================================\n");
	for(i = 0; i < N; i++)
		log_write("%lg\t",pi[i]);
	log_write("\n");
	log_write("===================================================================================================\n");
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++)
			log_write("%lg\t",A[i][j]);
		log_write("\n");
	}
	log_write("===================================================================================================\n");
	for(i = 0; i < N; i++){
		for(j = 0; j < M; j++)
			log_write("%lg\t",B[i][j]);
		log_write("\n");
	}
	log_write("===================================================================================================\n");
	log_write("forward probability : %lg\n", h -> __PROB__);
	log_write("P * : %lg\n", h -> __P_STAR__);
	log_write("State Sequance :");
	for(t = 0; t < T; t++)
		log_write("%d", h -> __Q_STAR__[t]);
	log_write("\n");
	log_write("===================================================================================================\n");
}
/*
	@param h is the hmm to be checked for correction(if the entries in the B matrix is empty make it non-zero)
	@return h is the modified hmm( B matrix with non-zero values)
*/
HMM * hmm_correction(HMM * h){
	h -> __L__ = lamda_correction(h -> __L__);
	return h;
}
/*
	@param obs_file name of the observation sequence file
	@hmm_file name of the hmm file which is to be constructed
*/
void hmm_train(const char * obs_file, const char * hmm_file){
	int * obs = NULL;
	int obs_len;
	long double p_old = 0;
	long double p_new = 0;
	HMM * h = NULL;
	obs = get_observation(obs, obs_file, &obs_len);	// generate observation sequence
	h = hmm_load(h,__DEFAULT_HMM__);	// load the default hmm
	h = hmm_init(h, obs_len);	// initialize the data structures
	do{
		p_old = p_new;
		h = update_model(h, obs, obs_len);	// update the model
		h = hmm_correction(h);	// if B matrix contain zero entries correct it
		h = viterbi(h, obs, obs_len);	// compute the best path sequence
		//hmm_print(h);	// dispaly the newly created hmm
		p_new = h -> __P_STAR__;
		//p_new = h -> __PROB__;
		//assert( h -> __PROB__ > 0);
		//log_write("probability:%lg\n",p_new);
	}while(p_new - p_old > 0);
	
	//h = hmm_correction(h);
	hmm_store(h,hmm_file);	// store the hmm
	//log_write("%s completed\n", hmm_file);
	free(obs);
	hmm_dispose(h);
}
/*
	@param hmm_file1 is the name of the hmm file which contains the final merged result
	@param hmm_file2 is the name of the hmm file which is to be merged with hmm_file1
*/
void hmm_merge(const char * hmm_file1, const char * hmm_file2){
	HMM * h1 = NULL;
	HMM * h2 = NULL;
	h1 = hmm_load(h1, hmm_file1);
	h2 = hmm_load(h2, hmm_file2);
	//hmm_print(h1);
	h1 -> __L__ = lamda_merge(h1 -> __L__, h2 -> __L__);
	hmm_store(h1, hmm_file1);
	//hmm_print(h1);
	lamda_dispose(h1 -> __L__);
	lamda_dispose(h2 -> __L__);
	free(h1);
	free(h2);
}
/*
	@param file_name is the input file which is to be recognized
*/
void hmm_test(const char * file_name){// recognize the given input file(digit)
	long double new_prob = 0;
	long double old_prob = 0;
	long double max_prob = 0;
	int * obs = NULL;
	int obs_len;
	HMM * h = NULL;
	FILE * data = NULL;
	data = file_open(data,HMM_CONF,"r");
	char line[BUFFER_SIZE];
	char output[BUFFER_SIZE] = "recognition failed";
	char * hmm_file = NULL;
	char * digit = NULL;
	char * name = NULL;
	obs = get_observation(obs, file_name, &obs_len);
	//h = hmm_load(h, __DEFAULT_HMM__);
	//h = hmm_init(h, obs_len);
	while(fgets(line, sizeof(line), data)){
		digit = strtok(line,":");
		//strtok(NULL,"\"");
		hmm_file = strtok(NULL,"\n");
		//log_write("%s\n",hmm_file);
		h = hmm_load(h,(const char *)hmm_file);
		h = hmm_init(h, obs_len);
		//log_write("HMM :%s\n",digit);
		//hmm_print(h);
		h = forward_procedure(h, obs, obs_len);
		//hmm_print(h);
		new_prob = h -> __PROB__;
		//log_write("%llg\n", new_prob);
		//log_write("%lg\n", new_prob);
		log_write("%s:%lg\n",digit, new_prob);
		if(new_prob > old_prob){
			strcpy(output, digit);
			//log_write("%s:%lg\n",digit, new_prob);
			old_prob = new_prob;
		}
		hmm_dispose(h);
	}
	//log_write("OUTPUT:%s\n",output);
	log_write("DIGIT RECOGNIZED:%s\n",output);
	free(obs);
}
/*
	@param h is the hmm to be disposed
*/
void hmm_dispose(HMM * h){
	int t;
	assert(h != NULL);
	lamda_dispose(h -> __L__);
	if(h -> __ALPHA__)	dispose_matrix(h -> __ALPHA__, h -> __T__, h -> __L__ ->__N__);
	if(h -> __BETA__)	dispose_matrix(h -> __BETA__,h -> __T__, h -> __L__ ->__N__);
	if(h -> __GAMMA__)	dispose_matrix(h -> __GAMMA__,h -> __T__, h -> __L__ ->__N__);
	if(h ->__ZETA__ ){
		for(t = 0; t < h -> __T__; t++)
			dispose_matrix(h ->__ZETA__[t], h -> __L__ ->__N__, h -> __L__ ->__N__);
		free(h -> __ZETA__);
	}
	if(h -> __Q_STAR__)	(h -> __Q_STAR__);
	free(h);
}