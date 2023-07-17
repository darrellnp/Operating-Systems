/*
 * file: preemptive.h
 */

#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__
#define MAXTHREADS 4

typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);
void delay(unsigned char);

int char_to_int(char);

unsigned char now(void);
#endif // __PREEMPTIVE_H__
