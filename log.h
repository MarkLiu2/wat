#ifndef LOG_H
#define LOG_H

#include<stdio.h>

#define LOG_INFO    1
#define LOG_ERROR   2
#define LOG_PANIC   3

void set_log_level(int);
int get_log_level();
void wat_log(int, char*);

#endif 
