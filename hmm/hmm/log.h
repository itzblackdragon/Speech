#ifndef	__LOG_H__
#define	__LOG_H__
#include<stdio.h>
#include"util.h"

extern Bool init_log();
extern void log_write(const char * format_string, ...);
extern void dispose_log();

#endif