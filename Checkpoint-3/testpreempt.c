/*
 * file: testpreempt.c
 */

#include <8051.h>
#include "preemptive.h"

#define CNAME(s) _ ## s
#define LABELNAME(label) label ## $

void SemaphoreCreate(char* s, char n) {
    __critical{
       *s = n;
    }
    return;
}

#define SemaphoreSignal(s) { \
   __asm \
      INC CNAME(s) \
   __endasm; \
}

#define SemaphoreWaitBody(s, label) { \
   __asm \
      LABELNAME(label): MOV ACC, CNAME(s) \
                        JZ LABELNAME(label) \
                        DEC CNAME(s) \
   __endasm; \
}

#define SemaphoreWait(s) { \
   SemaphoreWaitBody(s, __COUNTER__) \
}

__data __at (0x35) ThreadID curThreadID;
__data __at(0x36) char mutex;
__data __at(0x37) char full;
__data __at(0x38) char empty;
__data __at(0x39) char nextProd;
__data __at(0x3A) char head;
__data __at(0x3B) char tail;
__data __at(0x3D) char sharedBuf[3] = { ' ', ' ', ' ' };


void Producer(void) {
        nextProd = 'A';

        while (1) {
                SemaphoreWait(empty);
                SemaphoreWait(mutex);

                __critical{
                        sharedBuf[tail] = nextProd;
                        tail += 1;
                        if (tail == 3) tail = 0;
                        if (nextProd== 'Z') nextProd = 'A';
                        else nextProd += 1;
                }

                SemaphoreSignal(mutex);
                SemaphoreSignal(full);
        }
}

void Consumer(void) {
        TMOD |= 0x20;
        TH1 = (char)-6;
        SCON = 0x50;
        TR1 = 1;
        TI = 1;

        while (1) {
                SemaphoreWait(full);
                SemaphoreWait(mutex);

                __critical{
                        while (!TI) {}
                        SBUF = sharedBuf[head];
                        TI = 0;
                        head += 1;
                        if (head == 3) head = 0;
                }

                SemaphoreSignal(mutex);
                SemaphoreSignal(empty);
        }
}

void main(void) {
       SemaphoreCreate(&mutex, 1);
       SemaphoreCreate(&full, 0);
       SemaphoreCreate(&empty, 3);
       head = 0;
       tail = 0;
       curThreadID = ThreadCreate(Producer);

       __asm
            MOV 0x35, #48
            MOV sp, 0x30
       __endasm;

       Consumer();
}

void _sdcc_gsinit_startup(void) {
        __asm
            ljmp  _Bootstrap
        __endasm;
}

void _mcs51_genRAMCLEAR(void) {}
void _mcs51_genXINIT(void) {}
void _mcs51_genXRAMCLEAR(void) {}

void timer0_ISR(void) __interrupt(1) {
        __asm
            ljmp  _myTimer0Handler
        __endasm;
}
