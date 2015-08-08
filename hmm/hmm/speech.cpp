#include"util.h"
#include"config.h"
#include"frame.h"
#include"speech.h"
#include"log.h"
#include"vq.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<limits.h>
#include<assert.h>
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@return shift is the average shift
*/
float get_shift(FRAME * frame, int frame_count){
	int i;
	float shift = 0;
	for(i  = 0; i < frame_count; i++){
		//log_write("F[%d]: ",i);
		shift += SHIFT(frame[i]);
	}
	shift /= frame_count;
	//log_write("Average shift : %.3f\n",shift);
	return shift;
}
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@param shift is the amount of shift to be applied to each sample
	@return is the modified frame array
*/
FRAME * apply_shift(FRAME * frame, int frame_count,float shift){
	int i;
	for(i  = 0; i < frame_count; i++)
		frame[i] = SHIFT_APPLY(frame[i], shift);
	return frame;
}
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@return is the modified frame array
*/
FRAME * apply_window(FRAME * frame, int frame_count){
	int i;
	for(i  = 0; i < frame_count; i++)
		frame[i] = WINDOW_APPLY(frame[i]);
	return frame;
}
/*
	@param frames is the base adress of frame array
	@param silence_frames is the number of frames to be analysed to determine silence charecterestics
	@return IZCT is the ZCR threshold of silence region,  = MIN ( IF , ZCR_DASH + 2 * SD_ZCR)
*/
int compute_IZCT(FRAME * frames, int silence_frames){	// compute the ZCR threshold from the silence region
	int i;
	int ZCR = 0;	// Zero Crossing Rate.
	int SD_ZCR;	// Standerd Deviation of ZCR.
	int V_ZCR = 0;	// Variance of ZCR
	int ZCR_DASH;	// Mean of ZCR.
	int IZCT = 0;	// Zero Crossing Rate Threshold
	for (i = 0; i < silence_frames; i++){
		ZCR += frames[i].zcr;
	}
	ZCR_DASH = ZCR / silence_frames;
	for (i = 0; i < silence_frames; i++){
		V_ZCR += (int)pow((double)ZCR_DASH - frames[i].zcr, 2);
	}
	V_ZCR = V_ZCR / silence_frames;
	SD_ZCR = (int)sqrt((double)V_ZCR);
	//fprintf(log_f,"ZCR_DASH :%d\nSD_ZCR :%d\n", ZCR_DASH, SD_ZCR);
	IZCT = MIN(IF, ZCR_DASH + 2 * SD_ZCR);
	return IZCT;
}

/*
	
*/
float compute_ITL(FRAME * frames, int silence_frames){	// compute Lower Threshold in the silence region. 
	int i;
	float IMX = 0;	// Maximum value of magnitude in the silence region.
	float IMN = (float)INT_MAX;	// Minimum value of Magnitude in the silence region.
	float L1;	// L1 is a level which is 3 percentage of peak energy (Silence energy).  L1 = 0.3 * (IMAX - IMIN) + IMIN.
	float L2;	// L2 is 4 times the silence energy. L2 = 4 * IMIN.
	for (i = 0; i < silence_frames; i++){
		if (frames[i].avg_energy > IMX)
			IMX = frames[i].avg_energy;
		if (frames[i].avg_energy < IMN)
			IMN = frames[i].avg_energy;
	}
	L1 =(int) 0.3 * (IMX - IMN) + IMN;
	L2 = 4 * IMN;
	return MIN(L1,L2);	// ITL = MIN(L1 , L2).
	
}


void compute_thresholds(FRAME * frames, int silence_frames, int * IZCT, float * ITL, float * ITU){	// This function compute all the threshold values
	* ITL = compute_ITL(frames,silence_frames);
	* ITU = 5 * (*ITL);
	//fprintf(log_f,"ITL :%d\nITU :%d\nIZCT :%d\n",ITL, ITU, IZCT);
}

FRAME tag_frames(FRAME frame, float ITL, float ITU){
	if(frame.avg_energy < ITL)
		frame.type = SILENCE;
	else if(frame.avg_energy < ITU)
		frame.type = NOISE;
	else	frame.type = SPEECH;
	return frame;
}

/*
	@param frames is the starting address of the frame array
	@param filename is the name of the input file
	@param frame_size is the size of the frame
	@param frame_overlap is the amount of overlap between adjascent frames
	@param frame_count is the total number of frames in the frame array
	@return frames is the base address of the frame array
*/
FRAME * get_frames(FRAME * frames, const char * file_name, int * frame_count, int frame_size, int frame_overlap){
	int initial_size = 4;
	int alloc_length = 0;
	int logic_length = initial_size;
	FILE * data = NULL;
	data = file_open(data,file_name,"r");
	frames = (FRAME *)malloc(sizeof(FRAME) * initial_size);
	assert(frames != NULL);
	//log_write("F[%d] :",alloc_length);
	while(NEXT_FRAME(&data,&frames[alloc_length])){
		alloc_length++;
		if(alloc_length == logic_length){
			logic_length *= 2;
			frames = (FRAME *)realloc(frames, sizeof(FRAME) * logic_length);
			assert(frames != NULL);
		}
		//log_write("F[%d] :",alloc_length);
	}
	fclose(data);
	* frame_count = alloc_length;
	return frames;
}
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@return is the modified frame array
*/
FRAME * get_ZCR(FRAME * frame, int frame_count){
	int i;
	for(i = 0; i < frame_count; i++)
		frame[i].zcr = ZCR(frame[i]);
	return frame;
}
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@return is the modified frame array
*/
FRAME * get_avg_energy(FRAME * frame, int frame_count){
	int i;
	for(i = 0; i < frame_count; i++){
		frame[i].avg_energy = ENERGY(frame[i]);
		//log_write("F[%d] : %.3f\n",i,frame[i].avg_energy);
	}
	return frame;
}
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@return is the modified frame array
*/
FRAME * get_R(FRAME * frame, int frame_count){
	int i;
	//log_write("=============================================== Auto correlation vector ================================================\n");
	for(i = 0; i < frame_count; i++)
		frame[i] = R(frame[i]);
	return frame;
}
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@return is the modified frame array
*/
FRAME * get_alpha(FRAME * frame, int frame_count){
	int i;
	//log_write("=================================================== Alpha Values ======================================================\n");
	for(i = 0; i < frame_count; i++)
		frame[i] = ALPHA(frame[i]);
	return frame;
}
/*
	@param frame is the input frame array
	@param frame_count is the size of the frame array
	@return is the modified frame array
*/
FRAME * get_cepstral(FRAME * frame, int frame_count){
	int i;
	//log_write("=============================================== Cepstral Coefficients =================================================\n");
	for(i = 0; i < frame_count; i++)
		frame[i] = CEPSTRAL(frame[i]);
	return frame;
}
/*
	@param frames is the input frame array
	@param frame_county is the frame array size
	@param frame_size is the size of a frame
	@param low is the lowest sample value in the whole speech sample
	@param high is the highest sample value in the whole speech sample
*/
void get_range(FRAME * frames, int frame_count, float * low, float * high){
	int i;
	float min;
	float max;
	* low = (float)INT_MAX;
	* high = (float)INT_MIN;
	for(i = 0; i < frame_count; i++){
		GET_RANGE(frames[i], &min, &max);
		* low = MIN(min, * low);
		* high = MAX(max, * high);
	}
}
/*
	@param frames is the input frame array
	@param frame-count is the size of the frame array
	@param d_low is the lower bound of the destination range
	@param d_high is the upper bound of the destination range
	@return is the modified frame array with sample values in the destination range
*/
FRAME * normalize(FRAME * frames, int frame_count, float d_low, float d_high){
	int i;
	float min = 0;
	float max = 0;
	get_range(frames, frame_count, &min, &max);
	for(i = 0; i < frame_count; i++)
		frames[i] = MAP_TO_RANGE(frames[i], min, max, d_low, d_high);
	return frames;
}
/*
	@param frames is the input frame array
	@param frame_count is the frame array size
	@return frames is the frame array with weighted parameters
*/
FRAME * apply_cep_weight(FRAME * frames, int frame_count){
	int i;
	for(i = 0; i < frame_count; i++)
		frames[i] = CEP_WEIGHT(frames[i]);
	return frames;
}
extern void obs_store(FRAME * frames, const char * file_name, int frame_count){
	FILE * data = NULL;
	int i;
	data = file_open(data, file_name, "w");
	fprintf(data,"%d\n", frame_count);
	for(i = 0; i < frame_count; i++)
		fprintf(data, "%d ",frames[i].cluster);
}
