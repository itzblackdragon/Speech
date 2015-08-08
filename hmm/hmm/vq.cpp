#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"vq.h"
#include"log.h"
#include"util.h"
#include"config.h"
#include"frame.h"
#include"speech.h"
#include"codebook.h"
float ** code_book;//Code book
int cluster_count[CODE_BOOK_SIZE];//count the number of frames mapped to clusres

int binary_split(int m, int order);
FRAME * K_Mean(FRAME * frames, int frames_count, int K, int order);
FRAME * check_for_empty_cell(FRAME * frames,int frames_count, int m, int order);
void update_code_vector(FRAME * frames,int frames_count,int m, int order);
float compute_distortion(FRAME * frames,int frames_count);
void print_code_book(float ** cb,int size, int order);
FRAME * LBG(FRAME * frames, int frames_count, int order);
/*
	@param m is the current size of the codbook
	@param order is the size of the code vector
	@return 2 * m is the new codebook size
*/
int binary_split(int m, int order){
	int i, j;
	log_write("=================================================== Binary Split ===================================================\n");
	for(i = 0; i < order; i++){
		for(j = 0; j < m; j++){
			code_book[m + j][i] = code_book[j][i] * (float)(1 - EPSILON);	//y- = (1 - e) y
			code_book[j][i] *= (float)(1 + EPSILON);	// y+ = (1 + e) y
		}
	}
	print_code_book(code_book,2 * m,order);
	log_write("====================================================================================================================\n");
	return 2 * m;
}
/*
	@param frames is the input frame array
	@param frames_count is the size of the frame array
	@param m is the current size of the codebook
	@param order is the size of a code vector
*/
void update_code_vector(FRAME * frames, int frames_count, int m, int order){
	int i,j;
	for(i = 0; i < m; i++){
		for(j = 0; j < order; j++){
			code_book[i][j] = 0;	// set all codevectors to 0
		}
	}
	for(i = 0; i < frames_count; i++){
		for(j = 0; j < order; j++){
			code_book[frames[i].cluster][j] += frames[i].cepstrum[j];	// for each cluster,compute SIGMA C(i)
		}
	}
	for(i = 0; i < m; i++){
		for(j = 0; j < order; j++){
			if(cluster_count[i])
				code_book[i][j] /= cluster_count[i];	// for each cluster, devide the SIGMA C(i) by the size of the cluster. i.e, centroid 
		}
	}
	for(i = 0; i < m; i++){
		log_write("C[%d] : %d ",i,cluster_count[i]);
	}
	log_write("\n");
}
/*
	@param frames is the input frame array
	@param frames_count is the size of the frame array
	@return distrortion is the sum distances of each frame to its centroid
*/
float compute_distortion(FRAME * frames,int frames_count){
	int i;
	float distortion = 0;
	FILE * d_file = NULL;
	d_file = file_open(d_file,"output/distortion.csv","a+");
	for(i = 0; i < frames_count; i++)
		distortion += frames[i].distance;	// for all frames, compute sum of diviation from its centroid
	distortion /= frames_count;	// normalize total distortion by total frames
	fprintf(d_file,"%f\n", distortion);
	log_write("distortion : %f\n",distortion);
	file_close(d_file);
	return distortion;
}

/*
	@param frames is the input frame array
	@param farmes_count is the frame array size
	@param m is the current codebook size
	@param order is the code vector size
*/
FRAME * check_for_empty_cell(FRAME * frames,int frames_count, int m, int order){
	int i;
	int biggest = 0;	// index of largest size cluster
	int empty_cell = -1;	// there is no empty cell
	for(i = 0; i < m; i++){
		if(cluster_count[i] > biggest)
			biggest = i;
		if(cluster_count[i] == 0){
			empty_cell = i;	// Cluster i is empty
			log_write("Empty Cell : %d\n",i);
		}
	}
	if(empty_cell != -1){
		log_write("Resolving empty cell %d by splitting C[%d]\n",empty_cell, biggest);
		for(i = 0; i < ORDER; i++){
				code_book[empty_cell][i] = code_book[biggest][i] * (float)(1 - EPSILON);
				code_book[biggest][i] *= (float)(1 + EPSILON);
		}
		frames = K_Mean(frames, frames_count, m, order);
	}
	return frames;
}
FRAME * map_to_cluster(FRAME * frames, int frames_count,int K, int order){
	int i, j;
	float dist, dist_dash;	// tokura distances
	int cluster = 0;
	for(i = 0; i < frames_count; i++){
			//log_write("F[%d]::",i);
			dist = 999;	// a large value used to compare the tokhura distance from the first code vector
			for(j = 0; j < K; j++){
				dist_dash = tokhura_distance(code_book[j],frames[i].cepstrum, order); 
				//log_write("D[%d] : %f ",j,dist_dash);
				if(dist_dash < dist){
					dist = dist_dash;	// dist stores the minimal tokhura distance
					cluster = j;	// cluster is the index of the code vector to which the distance is minimal
				}
			}
			//log_write("\nminimum distance :%f\tcluster = %d\n",dist,cluster);
			frames[i].cluster = cluster;	// add frame to the cluster
			cluster_count[cluster] += 1;	// incriment the cluster size
			frames[i].distance = dist;	// store the distance from its centriod
	}
	return frames;
}
/*
	@param frames is the input frame array
	@param frames_count is the size of frame array
	@param K is the number of cluster to be formed
	@order is the size of code vector
	@return is the modified frame array
*/
FRAME * K_Mean(FRAME * frames, int frames_count,int K, int order){
	int i, iteration = 0;
	float d = 0, d_dash = 100;	// distrotions
	int cluster = 0;
	do{
		d = d_dash;	// d cointains previous distortion value
		for(i = 0; i < K; i++){	
			cluster_count[i] = 0;	// initialize K empty clusters
		}
		frames = map_to_cluster(frames,frames_count, K, order);
		update_code_vector(frames, frames_count, K, order);	// compute the centroid and upadate the code vector entry with that centroid
		d_dash = compute_distortion(frames, frames_count);	// compute the average distrortion 
		iteration++;
	}while(d - d_dash > 0);	// repeat until the change in distortion is zero
	log_write("=============================================== K Means Algorithm ==================================================\n");
	log_write("Cluster Size : %d\n", K);
	log_write("Number of iterations : %d\n", iteration);
	print_code_book(code_book,K,order);
	log_write("====================================================================================================================\n");
	if(frames_count > K)
		frames = check_for_empty_cell(frames,frames_count,K,order);	// check whether any empty cluster exist. if exist then solve it
	return frames;
}

/*
	@param frames is the frame array
	@param frames_count is the frame array size
	@param order is the code vector size
*/
FRAME * LBG(FRAME * frames, int frames_count, int order){
	int m = 1;	// start with code book of size 1
	cluster_count[0] = frames_count;	// initialy every frame is in cluster 0
	update_code_vector(frames, frames_count, m, order);	// compute the centroid and update it as code book entry
	print_code_book(code_book, m, order);
	//cluster_count[0] = 0;
	while(m < CODE_BOOK_SIZE){
		m = binary_split(m, order);
		frames = K_Mean(frames, frames_count, m, order);
	}
	return frames;
}

/*
	@param cb is the codebook
	@param size is the code book size
	@param order is the code vector size
*/
void print_code_book(float ** cb,int size, int order){
	int i,j;
	FILE * code_f = NULL;
	code_f = file_open(code_f,"output/codebook.csv","w");
	for(i = 0; i < size; i++){
		log_write("V[%d]:",i);
		for(j = 0; j < order; j++){
			log_write("%f\t",cb[i][j]);
			fprintf(code_f,"%f\t",cb[i][j]);
		}
		log_write("\n");
		fprintf(code_f,"\n");
	}
	file_close(code_f);
}
/*
	@param frames is the input frame(frame sequence of input file)
	@param file_name is the name of the observation file which is to be stored
*/
void store_observation(FRAME * frames, int frame_count, const char * file_name){
	int i;
	FILE * data = NULL;
	data = file_open(data, file_name,"w");
	assert(data != NULL);
	fprintf(data,"%d\n", frame_count);
	for(i = 0; i < frame_count; i++)
		fprintf(data, "%d ", frames[i].cluster);
	file_close(data);
}
/*
	@param file_name is the name of the trainig file
	@param codbook_file is the name of the codebook file which is to be generated
*/
void speech_train(const char * file_name, const char * codebook_file){
	int frames_count = 0;
	float shift = 0;
	FRAME * frames = NULL;
	CODEBOOK * cb = NULL;
	frames = get_frames(frames, file_name, &frames_count);
	if(frames_count){
		shift = get_shift(frames, frames_count);	// compute shift
		frames = apply_shift(frames, frames_count, shift);	// apply shift
		frames = normalize(frames, frames_count, DEST_LOW, DEST_HIGH);	// normalize the speech signal between the range -15000,+15000
		frames = apply_window(frames, frames_count);	// apply hamming window
		frames = get_ZCR(frames,frames_count);	// compute the ZCR value
		frames = get_avg_energy(frames,frames_count);	// compute the average energy
		frames = get_R(frames,frames_count);	// generate autocorelation matrix
		frames = get_alpha(frames,frames_count);	// compute LPC prediction coefficents
		frames = get_cepstral(frames,frames_count);	//	compute cepstral coefficients
		//frames = apply_cep_weight(frames, frames_count);	// apply window to cepstral coefficients
		cb = codebook_create(cb, CODE_BOOK_SIZE, ORDER);
		code_book = cb -> __CODEBOOK__;
		frames = LBG(frames,frames_count, cb -> __ORDER__);	// generate codebook
		codebook_store(cb, codebook_file);
		codebook_dispose(cb);
	}
	else	printf("ERROR : Insufficient data points!!\n");
	free(frames);
}
/*
	@param file_name is the name of the input file whose observaton sequence is to be generated
	@param codbook_file is the name of the codebook file
	@param obs_file is the name of the observation file which is to be generated
*/
void speech_test(const char * file_name,const char * codebook_file, const char * obs_file){
	int frames_count = 0;
	float shift = 0;
	FRAME * frames = NULL;
	CODEBOOK * cb = NULL;
	frames = get_frames(frames, file_name, &frames_count);
	if(frames_count){
		shift = get_shift(frames, frames_count);	// compute shift
		frames = apply_shift(frames, frames_count, shift);	// apply shift
		frames = normalize(frames, frames_count, DEST_LOW, DEST_HIGH);	// normalize the speech signal between the range -15000,+15000
		frames = apply_window(frames, frames_count);	// apply hamming window
		frames = get_ZCR(frames,frames_count);	// compute the ZCR value
		frames = get_avg_energy(frames,frames_count);	// compute the average energy
		frames = get_R(frames,frames_count);	// generate autocorelation matrix
		frames = get_alpha(frames,frames_count);	// compute LPC prediction coefficents
		frames = get_cepstral(frames,frames_count);	//	compute cepstral coefficients
		//frames = apply_cep_weight(frames, frames_count);	// apply window to cepstral coefficients
		cb = coadbook_load(cb, codebook_file);
		code_book = cb -> __CODEBOOK__;
		frames = map_to_cluster(frames,frames_count, cb -> __SIZE__ , cb -> __ORDER__);	// generate codebook
		store_observation(frames, frames_count, obs_file);
		codebook_dispose(cb);
	}
	else	printf("ERROR : Insufficient data points %s!!\n", file_name);
	free(frames);
}