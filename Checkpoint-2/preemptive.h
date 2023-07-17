/*
 * file: preemptive.h
 */

#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__
#define MAXTHREADS 2

typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);

#endif // __PREEMPTIVE_H__
