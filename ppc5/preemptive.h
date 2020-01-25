/*
 * File: preemptive.h
 * 
 * Author: Chen Kuan-Hung
 * Student ID: 105061171
 * Mail: khchen.me@gmail.com
 * 
 * Date: 2020/1/14
 * Course: Operating Systems (CS3423 Fall 2019)
 */

#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define MAXTHREADS 4  /* not including the scheduler */
/* the scheduler does not take up a thread of its own */

typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);

int char_to_int(char);

void delay(unsigned char);
unsigned char now(void);

#endif // __PREEMPTIVE_H__
