#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<assert.h>
#include<stdio.h>
#include"util.h"
#include"log.h"
#define	__TRUNCATE_LENGTH__		1024 * 1024	//100 MB

FILE * __log_ptr__;	// pointer to the log file
int	__INDEX__ = 0;	
Bool	__LOG_CREATED__ = FALSE;	// denote whether log file is craeted or not
char __file_name__[20];	// name of the log file

/*
	@param name is the log file name
*/
void get_name(char * name){
	sprintf(name,"log/log_%d.txt",__INDEX__);
	__INDEX__++;
}

/*
	init_log will create the log file
*/
Bool init_log(){
	if(!__LOG_CREATED__){
		get_name(__file_name__);
		__log_ptr__ = file_open(__log_ptr__,__file_name__,"w");
		__LOG_CREATED__ = TRUE;
	}
	else	fprintf(stderr,"Log was already created!!!\n");
	return __LOG_CREATED__;
}
/*
	@param format_string is the format string used for printing
	@... is the arguments
*/
void log_write(const char * format_string, ...){
	if(!__LOG_CREATED__){
		init_log();
	}
	va_list args;
	va_start(args, format_string);
	vfprintf(stdout,format_string,args);
	vfprintf(__log_ptr__,format_string,args);
	if(ftell(__log_ptr__) > __TRUNCATE_LENGTH__)
		dispose_log();
	va_end(args);
	
}
/*
	dispose_log() will close the file pointer
*/
void dispose_log(){
	fclose(__log_ptr__);
	__LOG_CREATED__ = FALSE;
}