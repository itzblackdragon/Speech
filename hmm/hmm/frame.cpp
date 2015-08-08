#include"frame.h"
#include"log.h"
#include<math.h>
#include<stdlib.h>
#include<assert.h>
/*
	@param frame is the input for which shift is computed
	@param frame_size the size of the frame
	@return shift is the average shift of input frame
*/
float compute_shift(FRAME frame, int frame_size){
	int i;
	float shift = 0;
	for (i = 0; i < frame_size; i++)
			shift += frame.samples[i];
	shift /= frame_size;
	//log_write("Shift : %f\n",shift);
	return shift;
}
/*
	@param frame is the input frame 
	@param frame_size the size of the frame
	@param shift is the shift to be applied
*/
FRAME apply_shift_frame(FRAME frame,float shift, int frame_size){
	int i;
	for (i = 0; i < frame_size; i++){
			frame.samples[i] -= shift;
			//log_write("%f\n",frame.samples[i]);
	}
	return frame;
}

/*
	@param frame is the frame to which window is applying
	@param frame_size the size of the frame
	@return frame is the windowed frame
*/
FRAME apply_window_frame(FRAME frame, int frame_size){
	int i;
	for (i = 0; i < frame_size; i++){
		frame.samples[i] = frame.samples[i] * compute_h(i,frame_size);	// S(i) = S(i) * W(i),	0 <= i < N
		//log_write("%f\n",frame.samples[i]);
	}
	return frame;
}
/*
	@param frame is the frame for which ZCR value is to be computed
	@param frame_size the size of the frame
	@return zcr is the zcr value of the frame
*/
int compute_ZCR(FRAME frame, int frame_size){
	int i;
	int ZCR = 0;
	Bool init_pos = FALSE;	// Assume initial sample value is negative.
	for(i = 0; i < frame_size; i++){
		if(init_pos && frame.samples[i] < 0){	// if zero crossing from +ve to negative
			init_pos = FALSE;	//change initial postion to -ve and incriment ZCR
			ZCR++;
		}
		if(!init_pos && frame.samples[i] > 0){	// if sample goes from - ve to +ve
			init_pos = TRUE;	// change initial postion to +ve and incriment ZCR
			ZCR++;
		}
	}
	//log_write("%d\n",ZCR);
	return ZCR;
}
/*
	@param frame is the frame for which energy is to be computed
	@param frame_size the size of the frame
	@return energy is average energy, E(n) = (1 / N) SIGMA SQUARE(S(i))
 */
float compute_avg_energy(FRAME frame, int frame_size){
	int i;
	float energy = 0;
	for (i = 0; i < frame_size; i++){
		energy += SQUARE(frame.samples[i]);	// E(i) = S(i) * S(i)
	}
	energy /= frame_size;
	return energy;
}
/*
	@param frame is the frame for which auto correlation is computed
	@param order is the order of prediction
	@param frame_size the size of the frame
	@return frame is the updated frame with R values R(i) = SIGMA S(n) * S(n + lag) , 0 <= i < N, 0 <= i,lag <= P
*/
FRAME compute_R(FRAME  frame,int frame_size, int order){
	int i,j;
	for (i = 0; i < order + 1; i++){
		frame.R[i] = 0;
		for (j = 0; j < frame_size - i; j++){
			frame.R[i] += frame.samples[j] * frame.samples[j + i];
		}
		//log_write("%f\t",frame.R[i]);
	}
	//log_write("\n");
	return frame;
}
/*
	@param frame is the frame for which prediction coefficient is to be computed
	@param order is the order of prediction
	@return frame is the frame with upadated Alapa values
*/
FRAME compute_alpha_coefficients(FRAME frame, int order){
	float * E = (float *)malloc(sizeof(float) * (order + 1));
	float * K = (float *)malloc(sizeof(float) * (order + 1));
	durbins_algorithm(E,K,frame.R,frame.alpha,order);
	free(E);
	free(K);
	return frame;
}
/*
	@param	frame is the frame for which cepstral coefficient is to be compuetd
	@param order is the order of prediction
	@return frame is the frame with upadated Cepstral values C(m) = A(m) + SIGMA (m / k) * A(m - k),  0 < m <= P, C(0) = ln R(0)
*/
FRAME compute_cepstral_coefficients(FRAME frame, int order){
	int m, k;
	float temp;
	temp = (float)log((double)frame.R[0]);
	FILE * cep = NULL;
	cep = file_open(cep,"output/cep_out.csv","a+");
	//log_write("%f\t",temp);	// we are not storing C[0] since it is not use in any analysis  
	fprintf(cep,"%f\t", temp);
	for (m = 1; m < order + 1; m++){
		temp = 0;
		for (k = 1; k < m; k++)
			temp += ((float)k / m) * frame.cepstrum[k - 1] * frame.alpha[m - k - 1];
		frame.cepstrum[m - 1] = frame.alpha[m - 1] + temp;	// we are storing C[m] physicaly at C[m - 1] since we are not considering C[0]
		//log_write("%f\t", frame.cepstrum[m - 1]);
		fprintf(cep,"%f\t",frame.cepstrum[m - 1]);
	}
	//log_write("\n");
	fprintf(cep,"\n");
	file_close(cep);
	return frame;
}
/*
	@param frame is the input frame
	@param order is the size of of the cepstral vector
	@return frame is the frame with weight applied.
*/
FRAME apply_cep_weight_frame(FRAME frame, int order){
	int i;
	for(i = 0; i < order; i++)
		frame.cepstrum[i] = cep_weight(i + 1, order) * frame.cepstrum[i];
	return frame;
}
/*
	@param file_ptr is the input file pointer
	@param frame is the input frame
	@param frame_size is the size of the frame
	@param frame_overlap is the amount of overlap between frames
	@return flag is an indication of success or failure
*/
Bool get_next_frame(FILE ** file_ptr,FRAME * frame, int frame_size, int frame_overlap){
	int i;
	long next_position = 0;	// starting position of next frame
	float value;
	Bool flag = TRUE;
	for (i = 0; i < frame_size; i++){
		if ((fscanf_s(* file_ptr, "%f", &value)) == 1){
			if(i == frame_overlap)
				next_position = ftell(* file_ptr);
			frame -> samples[i] = value;
				//log_write("%.3f ",value);
		}
		else flag = FALSE;	// frame cointains insufficient data point so discard the frame
	}
	frame -> cluster =  0;
	frame -> distance = 0;
	//log_write("\n");
	fseek(* file_ptr,next_position,SEEK_SET);	// update file pointer to start of next frame
	return flag;
}
/*
	@param frame is the input frame
	@param frame_size is the size of a single frame
	@param low is the minimum sample value in the given frame
	@param high is the maximum sample value in the given frame
*/
void get_range_frame(FRAME frame, int frame_size, float * low, float * high){
	int i;
	float min = (float)INT_MAX;
	float max = (float)INT_MIN;
	for(i = 0; i < frame_size; i++){
		if(frame.samples[i] > max)
			max = frame.samples[i];
		else if(frame.samples[i] < min)
			min = frame.samples[i];
	}
	* low = min;
	* high = max;
}
/*
	@param frame is the input frame
	@param frame_size is the size of the frame
	@param s_low is the lower bound of the whole speech sample
	@param s_high is the upper bound of the whole speech sample
	@param d_low is the lower bound of the destination range
	@param d_high is the upper bound of the destination range
	@return is the modified frame with all sample values mapped into the destination range
*/
FRAME map_to_range_frame(FRAME frame,int frame_size, float s_low, float s_high, float d_low, float d_high){
	int i;
	for(i = 0; i < frame_size; i++)
		frame.samples[i] = map_to_range(frame.samples[i], s_low, s_high, d_low, d_high);
	return frame;
}