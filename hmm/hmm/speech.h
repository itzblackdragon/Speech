#ifndef	__SPEECH_H__
#define __SPEECH_H__
#include"frame.h"
#define IF 25	//Fixed Threshold. We assume a fixed 25 zero crossing per 10 ms.
#define SILENCE_FRAMES 10	// Assume that the first 100ms does not contain any speech signal.
#define ZCR_TEST_FRAMES 5	//No. of frames considering for ZCR adjustment of boundary point. 
#define ZCR_COUNT_LIMIT 0.3 * ZCR_TEST_FRAMES	// Used to distinguish frecatives. Assume that freequency of ZCR in speech region is more than 10 its a frecative.
#define SPIKE_LENGTH 10 // Number of frames used to detect spike.

extern FRAME * get_frames(FRAME * frames, const char * file_name,int * frame_count, int frame_size = FRAME_SIZE, int frame_overlap = FRAME_OVERLAP);
extern FRAME * normalize(FRAME * frames, int frame_count, float d_low, float d_high);
extern float get_shift(FRAME * frame, int frame_count);
extern FRAME * apply_shift(FRAME * frame, int frame_count, float shift);
extern FRAME * apply_window(FRAME * frame, int frame_count);
extern FRAME * get_ZCR(FRAME * frame, int frame_count);
extern FRAME * get_avg_energy(FRAME * frame, int frame_count);
extern FRAME * get_R(FRAME * frame, int frame_count);
extern FRAME * get_alpha(FRAME * frame, int frame_count);
extern FRAME * get_cepstral(FRAME * frame, int frame_count);
extern FRAME * apply_cep_weight(FRAME * frames, int frame_count);
extern void obs_store(FRAME * frames, const char * file_name, int frame_count);
/*TODO:
extern int get_start_frame(FRAME * frame, int frame_count);
extern FRAME tag_frame(FRAME frame);
int detectStart(int startFrame);
int detectEnd(int endFrame);
int ZCRAdjustmentStart(int start);
int ZCRAdjustmentEnd(int end);
int findStartSample(int startFrame);
int findEndSample(int endFrame);
*/
#endif