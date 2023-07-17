/*
 * file: preemptive.c
 */

#include <8051.h>
#include "preemptive.h"
#define MAXTHREADS 4

//0x2*
__data __at (0x20) int oldSP;
__data __at (0x21) int newThreadSP;
__data __at (0x22) ThreadID newThreadID;
__data __at (0x23) char entryTime[5];
__data __at (0x28) char exitTime[5];
__data __at (0x2D) char c;
__data __at (0x2E) int thNum;

//0x3*
__data __at (0x30) int threads0;
__data __at (0x31) int threads1;
__data __at (0x32) int threads2;
__data __at (0x33) int threads3;
__data __at (0x34) int threadsBitMap;
__data __at (0x35) ThreadID curThreadID;
__data __at (0x36) char mutex;
__data __at (0x37) char slots[2];
__data __at (0x39) char delays[3];
__data __at (0x3C) char clock;
__data __at (0x3D) char temp4;
__data __at (0x3E) char temp5;

/*
 * @@@ [8 pts]
 * define a macro for saving the context of the current thread by
 * 1) push ACC, B register, Data pointer registers (DPL, DPH), PSW
 * 2) save SP into the saved Stack Pointers array
 *   as indexed by the current thread ID.
 * Note that 1) should be written in assembly,
 *     while 2) can be written in either assembly or C
 */
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
      case '2': \
         __asm \
            MOV 0x32, SP \
         __endasm; \
         break; \
      case '3': \
         __asm \
            MOV 0x33, SP \
         __endasm; \
         break; \
      default: \
         break; \
   } \
}
/*
 * @@@ [8 pts]
 * define a macro for restoring the context of the current thread by
 * essentially doing the reverse of SAVESTATE:
 * 1) assign SP to the saved SP from the saved stack pointer array
 * 2) pop the registers PSW, data pointer registers, B reg, and ACC
 * Again, popping must be done in assembly but restoring SP can be
 * done in either C or assembly.
 */
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
      case '2': \
         __asm \
            MOV SP, 0x32 \
         __endasm; \
         break; \
      case '3': \
         __asm \
            MOV SP, 0x33 \
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

void PrintParkingResult();

extern void main(void);
/*
 * Bootstrap is jumped to by the startup code to make the thread for
 * main, and restore its context so the thread can run.
 */
void Bootstrap(void) {
   //initialize
    TMOD = 0;
    IE = 0x82;
    TR0 = 1;

    threadsBitMap = 0x00;
    threads0 = 0x3F;
    threads1 = 0x4F;
    threads2 = 0x5F;
    threads3 = 0x6F;

    temp4 = 't';
    temp5 = 't';

    for (c = 0; c < 2; c++) {
        slots[c] = '_';
    }

    for (c = 0; c < 5; c++) {
        entryTime[c] = '_';
        exitTime[c] = '_';
    }

    curThreadID = ThreadCreate(main);

    clock = 96;
    thNum = 1;

    RESTORESTATE;
}
/*
 * ThreadCreate() creates a thread data structure so it is ready
 * to be restored (context switched in).
 * The function pointer itself should take no argument and should
 * return no argument.
 */
 //use a semaphore to allow creation of threads up to the max, and any attempt to create additional threads will block until some thread has exited.
ThreadID ThreadCreate(FunctionPtr fp) { //create new thread
     EA = 0;

     if ((threadsBitMap & 0xFF) == 0xFF){
        return -1;
     }

     newThreadID = 't';
     thNum++;

     if ((threadsBitMap & 0x01) == 0x00) {
        __asm
           MOV 0x22, #48
           ORL 0x34, #01
           MOV 0x21, 0x30
        __endasm;
     }
     else if ((threadsBitMap & 0x02) == 0x00) {
        __asm
           MOV 0x22, #49
           ORL 0x34, #02
           MOV 0x21, 0x31
        __endasm;
     }
     else if ((threadsBitMap & 0x04) == 0x00) {
         __asm
           MOV 0x22, #50
           ORL 0x34, #04
           MOV 0x21, 0x32
         __endasm;
     }
     else if ((threadsBitMap & 0x08) == 0x00) {
         __asm
           MOV 0x22, #51
           ORL 0x34, #08
           MOV 0x21, 0x33
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
           delays[0] = '0';
           break;
        case '2':
           __asm
              MOV PSW, #0x10
              PUSH PSW
              MOV 0x32, SP
           __endasm;
           delays[1] = '0';
           break;
        case '3':
           __asm
              MOV PSW, #0x18
              PUSH PSW
              MOV 0x33, SP
           __endasm;
           delays[2] = '0';
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
/*
 * this is called by a running thread to yield control to another
 * thread.  ThreadYield() saves the context of the current
 * running thread, picks another thread (and set the current thread
 * ID to it), if any, and then restores its state.
 */
void ThreadYield(void) {
    EA = 0;
    SAVESTATE;
   //find next available thread
    clock++;

    for (c = 0; c < 3 ; c++) {
         if (delays[c] > '0'){
                delays[c]--;
         }
    }

    if (mutex == 0) {
         if ((exitTime[0] == '_') && (delays[0] == '0')) {
            curThreadID = '1';
         }
         else if ((exitTime[1] == '_') && (delays[1] == '0')) {
            curThreadID = '2';
         }
         else if ((exitTime[2] == '_') && (delays[2] == '0')) {
            curThreadID = '3';
         }
         else if ((temp4 != 't') && (exitTime[3] == '_') && (delays[char_to_int(temp4)] == '0')) {
            curThreadID = temp4 + 1;
         }
         else if ((temp5 != 't') && (exitTime[4] == '_') && (delays[char_to_int(temp5)] == '0')) {
            curThreadID = temp5 + 1;
         }
         else if (exitTime[0] == '_') {
            curThreadID = '1';
         }
         else if (exitTime[1] == '_') {
            curThreadID = '2';
         }
         else if (exitTime[2] == '_') {
            curThreadID = '3';
         }
         else if (exitTime[3] == '_') {
            curThreadID = temp4 + 1;
         }
         else if (exitTime[4] == '_') {
            curThreadID = temp5 + 1;
         }
    }
    else {
         if (entryTime[0] == '_') {
            curThreadID = '1';
         }
         else if (entryTime[1] == '_') {
            curThreadID = '2';
         }
         else if (entryTime[2] == '_') {
            curThreadID = '3';
         }
         else if (entryTime[3] == '_') {
            curThreadID = temp4 + 1;
         }
         else if (entryTime[4] == '_') {
            curThreadID = temp5 + 1;
         }
         else if (exitTime[0] == '_') {
            curThreadID = '1';
         }
         else if (exitTime[1] == '_') {
            curThreadID = '2';
         }
         else if (exitTime[2] == '_') {
            curThreadID = '3';
         }
         else if (exitTime[3] == '_') {
            curThreadID = temp4 + 1;
         }
         else if (exitTime[4] == '_') {
            curThreadID = temp5 + 1;
         }
    }

    RESTORESTATE;
    EA = 1;
}

/*
 * ThreadExit() is called by the thread's own code to terminate
 * itself.  It will never return; instead, it switches context
 * to another thread.
 */
void ThreadExit(void) {
    thNum--;
    switch (curThreadID) {
        case '0':
            __asm
               ANL 0x34, #0xFE
            __endasm;
            break;
        case '1':
            __asm
               ANL 0x34, #0xFD
            __endasm;
            delays[0] = '-';
            break;
        case '2':
            __asm
               ANL 0x34, #0xFB
            __endasm;
            delays[1] = '-';
            break;
        case '3':
            __asm
               ANL 0x34, #0xF7
            __endasm;
            delays[2] = '-';
            break;
        default:
            break;
    }
   //empty
    if (curThreadID == '0') {
        EA = 0;
        PrintParkingResult();
        while (1) {}
    }
    else {
        if (temp4 == 't') {
            if(curThreadID == '1')
            {
               temp4 = '0';
            }
            else
            {
               temp4 = '1';
            }
        }
        else if (temp5 == 't') {
            if (curThreadID == '1') {
                temp5 = '0';
            }
            else if (curThreadID == '2') {
                temp5 = '1';
            }
            else if (curThreadID == '3') {
                temp5 = '2';
            }
        }
        curThreadID = '0';
    }

    RESTORESTATE;
}

void myTimer0Handler(void) {
//array of size 3 that saves the value of 3 delayed cars and that it will 
//decrements all 3 of them by 1 every 1 unit of clock time.
    EA = 0;
    SAVESTATE;

    clock++;

    for (c = 0; c < 3 ; c++) {
         if (delays[c] > '0'){
                delays[c]--; // delay decrement
         }
    }

    if (mutex == 0) {
         if ((exitTime[0] == '_') && (delays[0] == '0')) {
            curThreadID = '1';
         }
         else if ((exitTime[1] == '_') && (delays[1] == '0')) {
            curThreadID = '2';
         }
         else if ((exitTime[2] == '_') && (delays[2] == '0')) {
            curThreadID = '3';
         }
         else if ((temp4 != 't') && (exitTime[3] == '_') && (delays[char_to_int(temp4)] == '0')) {
            curThreadID = temp4 + 1;
         }
         else if ((temp5 != 't') && (exitTime[4] == '_') && (delays[char_to_int(temp5)] == '0')) {
            curThreadID = temp5 + 1;
         }
         else if (exitTime[0] == '_') {
            curThreadID = '1';
         }
         else if (exitTime[1] == '_') {
            curThreadID = '2';
         }
         else if (exitTime[2] == '_') {
            curThreadID = '3';
         }
         else if (exitTime[3] == '_') {
            curThreadID = temp4 + 1;
         }
         else if (exitTime[4] == '_') {
            curThreadID = temp5 + 1;
         }
    }
    else {
         if (entryTime[0] == '_') {
            curThreadID = '1';
         }
         else if (entryTime[1] == '_') {
            curThreadID = '2';
         }
         else if (entryTime[2] == '_') {
            curThreadID = '3';
         }
         else if (entryTime[3] == '_') {
            curThreadID = temp4 + 1;
         }
         else if (entryTime[4] == '_') {
            curThreadID = temp5 + 1;
         }
         else if (exitTime[0] == '_') {
            curThreadID = '1';
         }
         else if (exitTime[1] == '_') {
            curThreadID = '2';
         }
         else if (exitTime[2] == '_') {
            curThreadID = '3';
         }
         else if (exitTime[3] == '_') {
            curThreadID = temp4 + 1;
         }
         else if (exitTime[4] == '_') {
            curThreadID = temp5 + 1;
         }
    }

    RESTORESTATE;
    EA = 1;

    __asm
       RETI
    __endasm;
}

void delay(unsigned char c) {
      EA = 0;
      switch (curThreadID){
         case '1':
            delays[0] = c;
            break;
         case '2':
            delays[1] = c;
            break;
         case '3':
            delays[2] = c;
            break;
         default:
            break;
      }
      EA = 1;
}

unsigned char now(void) {
      return clock;
}

int char_to_int(char c) {
    return c - 48;
}

void PrintParkingResult() {
   while (!TI) {}
   SBUF = 'C';
   TI = 0;
   while (!TI) {}
   SBUF = 'a';
   TI = 0;
   while (!TI) {}
   SBUF = 'r';
   TI = 0;
   while (!TI) {}
   SBUF = '1';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   while (!TI) {}
   SBUF = entryTime[0];
   TI = 0;
   while (!TI) {}
   SBUF = '~';
   TI = 0;
   while (!TI) {}
   SBUF = exitTime[0] - 1;
   TI = 0;
   while (!TI) {}
   SBUF = '|';
   TI = 0;

   while (!TI) {}
   SBUF = 'C';
   TI = 0;
   while (!TI) {}
   SBUF = 'a';
   TI = 0;
   while (!TI) {}
   SBUF = 'r';
   TI = 0;
   while (!TI) {}
   SBUF = '2';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   while (!TI) {}
   SBUF = entryTime[1] - 1;
   TI = 0;
   while (!TI) {}
   SBUF = '~';
   TI = 0;
   while (!TI) {}
   SBUF = exitTime[1] - 2;
   TI = 0;
   while (!TI) {}
   SBUF = '|';
   TI = 0;

   while (!TI) {}
   SBUF = 'C';
   TI = 0;
   while (!TI) {}
   SBUF = 'a';
   TI = 0;
   while (!TI) {}
   SBUF = 'r';
   TI = 0;
   while (!TI) {}
   SBUF = '3';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   if ((entryTime[2] - exitTime[0]) == 1) {
      while (!TI) {}
      SBUF = entryTime[2] - 1;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = exitTime[2] - 2;
      TI = 0;
   } else if ((entryTime[2] - exitTime[1]) == 1) {
      while (!TI) {}
      SBUF = entryTime[2] - 2;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = exitTime[2] - 3;
      TI = 0;
   }
   while (!TI) {}
   SBUF = '|';
   TI = 0;

   while (!TI) {}
   SBUF = 'C';
   TI = 0;
   while (!TI) {}
   SBUF = 'a';
   TI = 0;
   while (!TI) {}
   SBUF = 'r';
   TI = 0;
   while (!TI) {}
   SBUF = '4';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   if ((entryTime[3] - exitTime[0]) == 1) {
      while (!TI) {}
      SBUF = entryTime[3] - 1;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = exitTime[3] - 2;
      TI = 0;
   } else if ((entryTime[3] - exitTime[1]) == 1){
      while (!TI) {}
      SBUF = entryTime[3] - 2;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = exitTime[3] - 3;
      TI = 0;
   } else if ((entryTime[3] - exitTime[2]) == 1) {
      if ((entryTime[2] - exitTime[0]) == 1) {
         while (!TI) {}
         SBUF = entryTime[3] - 2;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = exitTime[3] - 3;
         TI = 0;
      } else if ((entryTime[2] - exitTime[1]) == 1) {
         while (!TI) {}
         SBUF = entryTime[3] - 3;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = exitTime[3] - 4;
         TI = 0;
      }
   }
   while (!TI) {}
   SBUF = '|';
   TI = 0;

   while (!TI) {}
   SBUF = 'C';
   TI = 0;
   while (!TI) {}
   SBUF = 'a';
   TI = 0;
   while (!TI) {}
   SBUF = 'r';
   TI = 0;
   while (!TI) {}
   SBUF = '5';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   if ((entryTime[4] - exitTime[0]) == 1) {
      while (!TI) {}
      SBUF = entryTime[4] - 1;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = exitTime[4] - 2;
      TI = 0;
   } else if ((entryTime[4] - exitTime[1]) == 1){
      while (!TI) {}
      SBUF = entryTime[4] - 2;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = exitTime[4] - 3;
      TI = 0;
   } else if ((entryTime[4] - exitTime[2]) == 1) {
      if ((entryTime[2] - exitTime[0]) == 1) {
         while (!TI) {}
         SBUF = entryTime[4] - 2;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = exitTime[4] - 3;
         TI = 0;
      } else if ((entryTime[2] - exitTime[1]) == 1) {
         while (!TI) {}
         SBUF = entryTime[4] - 3;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = exitTime[4] - 4;
         TI = 0;
      }
   } else if ((entryTime[4] - exitTime[3]) == 1) {
      if ((entryTime[3] - exitTime[0]) == 1) {
         while (!TI) {}
         SBUF = entryTime[4] - 2;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = exitTime[4] - 3;
         TI = 0;
      } else if ((entryTime[3] - exitTime[1]) == 1) {
         while (!TI) {}
         SBUF = entryTime[4] - 3;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = exitTime[4] - 4;
         TI = 0;
      } else if ((entryTime[3] - exitTime[2]) == 1) {
         if ((entryTime[2] - exitTime[0]) == 1) {
            while (!TI) {}
            SBUF = entryTime[4] - 3;
            TI = 0;
            while (!TI) {}
            SBUF = '~';
            TI = 0;
            while (!TI) {}
            SBUF = exitTime[4] - 4;
            TI = 0;
         } else if ((entryTime[2] - exitTime[1]) == 1){
            while (!TI) {}
            SBUF = entryTime[4] - 4;
            TI = 0;
            while (!TI) {}
            SBUF = '~';
            TI = 0;
            while (!TI) {}
            SBUF = exitTime[4] - 5;
            TI = 0;
         }
      }
   }
   while (!TI) {}
   SBUF = '|';
   TI = 0;
}
