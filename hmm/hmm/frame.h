#ifndef	__FRAME_H__
#define __FRAME_H__
#include<stdio.h>
#include"config.h"
#include"util.h"
//definition of frame type
typedef enum{
	SILENCE,
	NOISE,
	SPEECH
}FRAME_TYPE;

//definition of frame
typedef struct{
	float samples[FRAME_SIZE];	// samples of a frame
	float avg_energy;	// average magnitude of the frame
	int zcr;	// zero crossing rate for the frame
	FRAME_TYPE type;	// silence , noise or speech
	float R[ORDER + 1];	// Auto correlation vector (R[0] ... R[P])
	float alpha[ORDER];//predictor coefficients (alpha[1] .. alpha[P-1])
	float cepstrum[ORDER];//cepstral coefficients (c[0],c[1] ... c[P])
	int cluster;	// cluster to which the frame belongings to
	float distance;	// distance from the centroid of the cluster
}FRAME;

extern Bool get_next_frame(FILE ** file_ptr,FRAME * frame, int frame_size, int frame_overlap);
extern float compute_shift(FRAME frame, int frame_size);
extern FRAME apply_shift_frame(FRAME frame,float shift, int frame_size);
extern FRAME apply_window_frame(FRAME frame, int frame_size);
extern int compute_ZCR(FRAME frame, int frame_size);
extern float compute_avg_energy(FRAME frame, int frame_size);
extern FRAME compute_R(FRAME frame,int frame_size, int order);
extern FRAME compute_alpha_coefficients(FRAME frame, int order);
extern FRAME compute_cepstral_coefficients(FRAME frame, int order);
extern FRAME apply_cep_weight_frame(FRAME frame, int order);
extern void get_range_frame(FRAME frame, int frame_size,float * low, float * high);
extern FRAME map_to_range_frame(FRAME frame,int frame_size, float s_low, float s_high, float d_low, float d_high);

#define NEXT_FRAME(file_ptr, frame_ptr)	\
	get_next_frame(file_ptr, frame_ptr, FRAME_SIZE, FRAME_OVERLAP)
#define SHIFT(frame)	\
		compute_shift(frame, FRAME_SIZE)
#define SHIFT_APPLY(frame, shift)	\
	apply_shift_frame(frame, shift, FRAME_SIZE)
#define WINDOW_APPLY(frame)	\
	apply_window_frame(frame, FRAME_SIZE)
#define ZCR(frame)	\
	compute_ZCR(frame, FRAME_SIZE)
#define ENERGY(frame)	\
	compute_avg_energy(frame, FRAME_SIZE)
#define R(frame)	\
	compute_R(frame, FRAME_SIZE, ORDER)
#define ALPHA(frame)	\
	compute_alpha_coefficients(frame, ORDER)
#define CEPSTRAL(frame)	\
	compute_cepstral_coefficients(frame, ORDER)
#define CEP_WEIGHT(frame)	\
	apply_cep_weight_frame(frame, ORDER)
#define GET_RANGE(frame, low, high)	\
	get_range_frame(frame, FRAME_SIZE, low, high)
#define MAP_TO_RANGE(frame, s_low, s_high, d_low, d_high)	\
	map_to_range_frame(frame, FRAME_SIZE, s_low, s_high, d_low, d_high)
#endif