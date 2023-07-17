/*
 * file: test3thread.c
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
__data __at (0x36) char mutex;
__data __at (0x37) char full;
__data __at (0x38) char empty;
__data __at (0x39) char nextProdAlp;
__data __at (0x3A) char nextProdNum;
__data __at (0x3B) char head;
__data __at (0x3C) char tail;
__data __at (0x3D) char sharedBuf[3] = {' ', ' ', ' '};


void ProducerAlp(void) {
        nextProdAlp = 'A';

        while (1) {
                SemaphoreWait(empty);
                SemaphoreWait(mutex);
                sharedBuf[tail] = nextProdAlp;
                tail += 1;
                if (tail == 3) tail = 0;
                if (nextProdAlp == 'Z') nextProdAlp = 'A';
                else nextProdAlp += 1;
                SemaphoreSignal(mutex);
                SemaphoreSignal(full);
        }
}

void ProducerNum(void) {
        nextProdNum = '0';

        while (1) {
                SemaphoreWait(empty);
                SemaphoreWait(mutex);
                sharedBuf[tail] = nextProdNum;
                tail += 1;
                if (tail == 3) tail = 0;
                if (nextProdNum == '9') nextProdNum = '0';
                else nextProdNum += 1;
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
                while (!TI) {}
                SBUF = sharedBuf[head];
                TI = 0;
                head += 1;
                if (head == 3) head = 0;
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
       curThreadID = ThreadCreate(ProducerAlp);
       curThreadID = ThreadCreate(ProducerNum);

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
