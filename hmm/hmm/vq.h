#ifndef __VECT_QUANT_H__
#define __VECT_QUANT_H__
#define __DEFAULT_NAME__ "temp/codebook.cb"
#include"frame.h"
extern void speech_train(const char * file_name, const char * codebook_file);
extern void speech_test(const char * file_name, const char * codebook_file, const char * obs_file);
#define SPEECH_TRAIN(input_file)	\
	speech_train(input_file, DEFAULT_CODE_BOOK)
#define SPEECH_TEST(test_input)	\
	speech_test(test_input, DEFAULT_CODE_BOOK, DEFAULT_OBS)
#endif