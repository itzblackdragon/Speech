#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEBUG 1	//Mode
#define FRAME_SIZE 320 // Considering a frame of size 20ms.
#define FRAME_OVERLAP FRAME_SIZE / 4	//	Overlap between to adjascent frames
#define ORDER 12	//	Order of prediction(P)
#define CODE_BOOK_SIZE 32	// size of the final code book
#define EPSILON	0.03	// epsilon value used by binary spliting algorithm.
#define DEST_LOW -15000	// lower range of speech signal
#define DEST_HIGH 15000	//upper range of speech singnal
#define DEFAULT_CODE_BOOK "temp/codebook.cb"
#define DEFAULT_OBS	"temp/observation.obs"
#define DEFAULT_INPUT "train/train.txt"
#define DEFAULT_HMM "input/test.hmm"
#endif