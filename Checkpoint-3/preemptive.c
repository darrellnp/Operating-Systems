/*
 * file: preemptive.c
 */

#include <8051.h>
#include "preemptive.h"
#define MAXTHREADS 2

__data __at (0x20) int oldSP;
__data __at (0x21) int newThreadSP;
__data __at (0x22) ThreadID newThreadID;
__data __at (0x30) int th0_SP;
__data __at (0x31) int th1_SP;
__data __at (0x34) int thBitMap;
__data __at (0x35) ThreadID curThreadID;

#define SAVESTATE { \
   __asm \
      PUSH ACC \
      PUSH B \
      PUSH DPL \
      PUSH DPH \
      PUSH PSW \
   __endasm; \
   switch (curThreadID) { \
      case '0': \
         __asm \
            MOV 0x30, SP \
         __endasm; \
         break; \
      case '1': \
         __asm \
            MOV 0x31, SP \
         __endasm; \
         break; \
      default: \
         break; \
   } \
}

#define RESTORESTATE { \
   switch (curThreadID) { \
      case '0': \
         __asm \
            MOV SP, 0x30 \
         __endasm; \
         break; \
      case '1': \
         __asm \
            MOV SP, 0x31 \
         __endasm; \
         break; \
      default: \
         break; \
   } \
   __asm \
      POP PSW \
      POP DPH \
      POP DPL \
      POP B \
      POP ACC \
   __endasm; \
}

extern void main(void);

void Bootstrap(void) {
    TMOD = 0;
    IE = 0x82;
    TR0 = 1;
    thBitMap = 0x00;
    th0_SP = 0x3F;
    th1_SP = 0x4F;
    curThreadID = ThreadCreate(main);
    RESTORESTATE;
}

ThreadID ThreadCreate(FunctionPtr fp) {
     EA = 0;

     if ((thBitMap & 0x09) == 0x09){
        return -1;
     }

     newThreadID = 't';

     if ((thBitMap & 0x01) == 0x00) {
        __asm
           MOV 0x22, #48
           ORL 0x34, #0x01
           MOV 0x21, 0x30
        __endasm;
     }
     else if ((thBitMap & 0x08) == 0x00) {
        __asm
           MOV 0x22, #49
           ORL 0x34, #0x08
           MOV 0x21, 0x31
        __endasm;
     }

     __asm
        MOV 0x20, sp
        MOV sp, 0x21
     __endasm;

     __asm
        PUSH DPL
        PUSH DPH
     __endasm;

     __asm
        MOV A, 0x00
        PUSH ACC
        PUSH B
        PUSH DPL
        PUSH DPH
     __endasm;

     switch (newThreadID) {
        case '0':
           __asm
              MOV PSW, #0x00
              PUSH PSW
              MOV 0x30, SP
           __endasm;
           break;
        case '1':
           __asm
              MOV PSW, #0x08
              PUSH PSW
              MOV 0x31, SP
           __endasm;
           break;
        default:
           break;
     }

     __asm
        MOV sp, 0x20
     __endasm;

     EA = 1;

     return newThreadID;
}

void ThreadYield(void) {
    EA = 0;
    SAVESTATE;

    do {
        switch (curThreadID) {
            case '0':
               curThreadID = '1';
               break;
            case '1':
               curThreadID = '0';
               break;
            default:
               break;
         }

         if ((curThreadID == '0') && ((thBitMap & 0x01) == 0x01)) {
            break;
         }
         else if ((curThreadID == '1') && ((thBitMap & 0x08) == 0x08)) {
            break;
         }
    }

    while (1);

    RESTORESTATE;
    EA = 1;
}

void ThreadExit(void) {
    EA = 0;
    RESTORESTATE;
    EA = 1;
}

void myTimer0Handler(void){
    EA = 0;
    SAVESTATE;

    do {
        switch (curThreadID) {
            case '0':
               curThreadID = '1';
               break;
            case '1':
               curThreadID = '0';
               break;
            default:
               break;
         }

         if ((curThreadID == '0') && ((thBitMap & 0x01) == 0x01)) {
            break;
         }
         else if ((curThreadID == '1') && ((thBitMap & 0x08) == 0x08)) {
            break;
         }

    }
    while (1);

    RESTORESTATE;
    EA = 1;
    __asm
        RETI
    __endasm;
}
