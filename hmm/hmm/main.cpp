#include<stdio.h>
#include<stdlib.h>
#include"frame.h"
#include"speech.h"
#include"config.h"
#include"log.h"
#include"vq.h"
#include"util.h"
#include"log.h"
#include"lamda.h"
#include"hmm.h"
/*
	@param name is the name to be constructed
	@param base is the path to the base directory
	@param folder is the name of the folder inside the base directory
	@param file is the name of the file in the given folder
	@param extenstion is the extenstion of the file
*/
void get_name(char * name,char * base,int folder,int file,char * extension){// create the path to a file
	sprintf(name,"%s/%d/%d%s", base, folder,file, extension);
}
/*
	@param name is the name to be constructed
	@param hmm_base is the path to the base directory
	@param folder is the name of the folder inside the base directory
*/
void get_hmm_default(char * name,char * hmm_base, int folder){	// create path to hmm default file
	sprintf(name,"%s/%d/%s", hmm_base, folder,"default.hmm");
}
/*
	@param hmm_base name of the base directory to which hmm file to be stored
	@param train_base name of the base directory from which input file to be fetched
*/
void train(char * hmm_base, char * train_base){	// create models
	char file_name[30];
	char hmm_name[30];
	char * test_obs = "temp/test.obs";
	int i, j;
	for(i = 0; i < 10; i++){
		get_name(file_name, train_base, i, 0, ".txt");
		//log_write("generating observation for file %s\n",file_name);
		SPEECH_TEST(file_name);
		//log_write("testing file %s\n",file_name);
		get_hmm_default(hmm_name,hmm_base, i);
		log_write("training  %s using %s\n",hmm_name, file_name);
		hmm_train(DEFAULT_OBS, hmm_name);
		for(j = 1; j <= 30; j++){
			get_name(file_name, train_base, i, j, ".txt");
			//log_write("generating observation for file %s\n",file_name);
			SPEECH_TEST(file_name);
			get_name(hmm_name, hmm_base, i, j,".hmm");
			log_write("training  %s using %s\n",hmm_name, file_name);
			hmm_train(DEFAULT_OBS, hmm_name);
			get_hmm_default(file_name,hmm_base, i);
			//log_write("opening file %s\n",hmm_name);
			hmm_merge(file_name, hmm_name);
			//log_write("upadating %s using %s \n", file_name, hmm_name);
		}
		//hmm_test(test_obs);
		log_write("Training completed completed %d\n", i);
	}

}
//start of main
int main(){
	char * hmm_base ="temp/hmm";
	char * train_base = "train";
	init_log();
	int i, j;
	int choice;
	int flag = 1;
	char file_name[100];
	do{
		printf("\nMENU\n1.TRAIN.\n2.TEST.\n3.QUIT.\n");
		scanf("%d", &choice);
		switch(choice){
		case 1 :
			printf("\nTraining started\nThis will take few minuts depending on the size of the input....");
			SPEECH_TRAIN("input/train.txt");
			printf("\nTraining completed.\n");
			train(hmm_base, train_base);
			break;
		case 2 :
			printf("Enter the input file :");
			scanf("%s", file_name);
			SPEECH_TEST(file_name);
			hmm_test(DEFAULT_OBS);
			break;
		case 3 :	
			flag = 0;
			break;	
		default :
			printf("Invalid choice!!!\n");
		}
	}while(flag);
/*	printf("Training started\nThis will take few minuts depending on the size of the input....");
	speech_train("input/data.txt", "temp/test.cb");
	printf("\nTraining completed.\n");
	speech_test("input/0.txt","temp/test.cb","temp/test.obs");
	hmm_train("temp/test.obs", "temp/test.hmm");
*/
	dispose_log();
	getchar();
	return 0;
}