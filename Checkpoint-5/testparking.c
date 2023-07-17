/*
 * file: testparking.c
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
// 0x2*
__data __at (0x23) char entryTime[5];
__data __at (0x28) char exitTime[5];
__data __at (0x2D) char c;
__data __at (0x2E) int thNum;

// 0x3*
__data __at (0x35) ThreadID curThreadID;
__data __at (0x36) char mutex;
__data __at (0x37) char slots[2];
__data __at (0x39) char delays[3];
__data __at (0x3C) char clock;
__data __at (0x3D) int th4;
__data __at (0x3E) int th5;

int _compare(char *a, char b) {return (*a == b);}

void Parking1(void) {
    while (1) {
        while (_compare(&delays[0], '0') == 0) {
            ThreadYield();
        }

        for (c = 0; c < 2; c++) {
            if (_compare(&slots[c], '1') == 1) {
                slots[c] = '_';
                exitTime[0] = now();
                SemaphoreSignal(mutex);
                ThreadExit();
            }
        }
        for (c = 0; c < 2; c++) {
            if (_compare(&slots[c], '_') == 1) {
                SemaphoreWait(mutex);
                slots[c] = '1';
                delay('8');
                entryTime[0] = now();
                break;
            }
            if (c == 1) delay('2');
        }
    }
}

void Parking2(void) {
        while (1) {
            while (_compare(&delays[1], '0') == 0) {
                ThreadYield();
            }

            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '2') == 1) {
                    slots[c] = '_';
                    exitTime[1] = now();
                    SemaphoreSignal(mutex);
                    ThreadExit();
                }
            }
            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '_') == 1) {
                    SemaphoreWait(mutex);
                    slots[c] = '2';
                    delay('5');
                    entryTime[1] = now();
                    break;
                }
                if (c == 1) delay('2');
            }
        }
}
void Parking3(void) {
        while (1) {
            while (_compare(&delays[2], '0') == 0) {
                ThreadYield();
            }

            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '3') == 1) {
                    slots[c] = '_';
                    exitTime[2] = now();
                    SemaphoreSignal(mutex);
                    ThreadExit();
                }
            }
            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '_') == 1) {
                    SemaphoreWait(mutex);
                    slots[c] = '3';
                    delay('5');
                    entryTime[2] = now();
                    break;
                }
                if (c == 1) delay('2');
            }
        }
}
void Parking4(void) {
        while (1) {
            while (_compare(&delays[char_to_int(th4)], '0') == 0) {ThreadYield();}
            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '4') == 1) {
                    slots[c] = '_';
                    exitTime[3] = now();
                    SemaphoreSignal(mutex);
                    ThreadExit();
                }
            }
            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '_') == 1) {
                    SemaphoreWait(mutex);
                    slots[c] = '4';
                    delay('7');
                    entryTime[3] = now();
                    break;
                }
                if (c == 1) delay('2');
            }
        }
}
void Parking5(void) {
        while (1) {
            while (_compare(&delays[char_to_int(th5)], '0') == 0) {ThreadYield();}
            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '5') == 1) {
                    slots[c] = '_';
                    exitTime[4] = now();
                    SemaphoreSignal(mutex);
                    ThreadExit();
                }
            }
            for (c = 0; c < 2; c++) {
                if (_compare(&slots[c], '_') == 1) {
                    SemaphoreWait(mutex);
                    slots[c] = '5';
                    delay('2');
                    entryTime[4] = now();
                    break;
                }
                if (c == 1) delay('2');
            }
        }
}

void main(void) {
        SemaphoreCreate(&mutex, 2);

        ThreadCreate(Parking1);
        ThreadCreate(Parking2);
        ThreadCreate(Parking3);

        TMOD |= 0x20;
        TH1 = (char)-6;
        SCON = 0x50;
        TR1 = 1;
        TI = 1;

        ThreadYield();

        while (thNum >= MAXTHREADS) {ThreadYield();}
        ThreadCreate(Parking4);

        while (thNum >= MAXTHREADS) {ThreadYield();}
        ThreadCreate(Parking5);

        while (thNum != 1) {ThreadYield();}

        ThreadExit();
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
