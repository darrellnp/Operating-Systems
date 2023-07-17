/*
 * file: testpreempt.c
 */

#include <8051.h>
#include "preemptive.h"

__data __at (0x35) ThreadID curThreadID;
__data __at (0x3D) char nextProd;
__data __at (0x3E) char sharedBuf;
__data __at (0x3F) int bufAvail;


void Producer(void) {
        nextProd = 'A';

        while (1) {
                if (bufAvail == 1){}
                else{
                    sharedBuf = nextProd;

                    if (nextProd == 'Z'){
                        nextProd = 'A';
                    }
                    else{
                        nextProd += 1;
                    }

                    bufAvail = 1;
                }
        }
}

void Consumer(void) {
        TMOD |= 0x20;
        TH1 = (char)-6;
        SCON = 0x50;
        TR1 = 1;
        TI = 1;

        while (1) {
                 if (bufAvail == 0){}
                 else{
                    while (!TI) {}
                    SBUF = sharedBuf;
                    TI = 0;
                    bufAvail = 0;
                 }
        }
}

void main(void) {
       sharedBuf = ' ';
       bufAvail = 0;
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
