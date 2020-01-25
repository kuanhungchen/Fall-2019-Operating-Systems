/* 
 * file: testparking.c 
 */
#include <8051.h>
#include "preemptive.h"

#define CNAME(s) _ ## s
#define LABELNAME(label) label ## $

void SemaphoreCreate(char *s, char n) {
        /* create a semaphore */
        __critical {
                *s = n;
        }
        return;
}

#define SemaphoreSignal(s) { \
        /* release a semaphore */ \
        __asm \
        INC CNAME(s) \
        __endasm; \
}

#define SemaphoreWaitBody(s, label) { \
        /* main part for decrementing a semaphore */ \
        __asm \
        LABELNAME(label): MOV ACC, CNAME(s) \
                          JZ LABELNAME(label) \
                          DEC CNAME(s) \
        __endasm; \
}

#define SemaphoreWait(s) { \
        /* driver for decrementing a semaphore */ \
        SemaphoreWaitBody(s, __COUNTER__) \
}

int _compare(char *s, char t) {return (*s == t);}

__data __at (0x23) char EnterTimes[5];
__data __at (0x28) char LeaveTimes[5];

__data __at (0x2D) char i;
__data __at (0x2E) int numOfThreads;

__data __at (0x35) ThreadID currentThreadID;
__data __at (0x36) char mutex;
__data __at (0x37) char spots[2];
__data __at (0x39) char delays[3];
__data __at (0x3C) char Time;
__data __at (0x3D) int thrd_4;
__data __at (0x3E) int thrd_5;

void Parking1(void) {
        /* car 1 for parking lot example */
        while (1) {
                while (_compare(&delays[0], '0') == 0) {ThreadYield();}
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '1') == 1) {
                                // already in parking lot, leave now
                                spots[i] = '_';
                                LeaveTimes[0] = now();
                                SemaphoreSignal(mutex);
                                ThreadExit();
                        }
                }
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '_') == 1) {
                                // find out a parking lot, enter now
                                SemaphoreWait(mutex);
                                spots[i] = '1';                                
                                delay('8');
                                EnterTimes[0] = now();
                                break;
                        }
                        if (i == 1) delay('2');
                }
        }        
}
void Parking2(void) {
        /* car 2 for parking lot example */
        while (1) {
                while (_compare(&delays[1], '0') == 0) {ThreadYield();}
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '2') == 1) {
                                // already in parking lot, leave now
                                spots[i] = '_';
                                LeaveTimes[1] = now();
                                SemaphoreSignal(mutex);
                                ThreadExit();
                        }                        
                }
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '_') == 1) {
                                // find out a parking lot, enter now
                                SemaphoreWait(mutex);
                                spots[i] = '2';
                                delay('4');
                                EnterTimes[1] = now();
                                break;
                        }
                        if (i == 1) delay('2');
                }
        }
}
void Parking3(void) {
        /* car 3 for parking lot example */
        while (1) {
                while (_compare(&delays[2], '0') == 0) {ThreadYield();}
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '3') == 1) {
                                // already in parking lot, leave now
                                spots[i] = '_';
                                LeaveTimes[2] = now();
                                SemaphoreSignal(mutex);
                                ThreadExit();
                        }                        
                }
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '_') == 1) {
                                // find out a parking lot, enter now
                                SemaphoreWait(mutex);
                                spots[i] = '3';
                                delay('5');
                                EnterTimes[2] = now();
                                break;
                        }
                        if (i == 1) delay('2');
                }
        } 
}
void Parking4(void) {
        /* car 4 for parking lot example */
        while (1) {
                while (_compare(&delays[char_to_int(thrd_4)], '0') == 0) {ThreadYield();}
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '4') == 1) {
                                // already in parking lot, leave now
                                spots[i] = '_';
                                LeaveTimes[3] = now();
                                SemaphoreSignal(mutex);
                                ThreadExit();
                        }                        
                }
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '_') == 1) {
                                // find out a parking lot, enter now
                                SemaphoreWait(mutex);
                                spots[i] = '4';
                                delay('7');
                                EnterTimes[3] = now();
                                break;
                        }
                        if (i == 1) delay('2');
                }                
        }        
}
void Parking5(void) {
        /* car 5 for parking lot example */
        while (1) {
                while (_compare(&delays[char_to_int(thrd_5)], '0') == 0) {ThreadYield();}
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '5') == 1) {
                                // already in parking lot, leave now
                                spots[i] = '_';
                                LeaveTimes[4] = now();
                                SemaphoreSignal(mutex);
                                ThreadExit();
                        }
                }
                for (i=0; i<2; i++) {
                        if (_compare(&spots[i], '_') == 1) {
                                // find out a parking lot, enter now
                                SemaphoreWait(mutex);
                                spots[i] = '5';
                                delay('2');
                                EnterTimes[4] = now();
                                break;
                        }
                        if (i == 1) delay('2');
                }                
        }        
}

void main(void) {
        /* main component for parking lot example */
        SemaphoreCreate(&mutex, 2);  // semaphore for spots

        ThreadCreate(Parking1);
        ThreadCreate(Parking2);
        ThreadCreate(Parking3);

        TMOD |= 0x20;
        TH1 = (char)-6;
        SCON = 0x50;
        TR1 = 1;
        TI = 1;
        
        ThreadYield();  // start parking

        while (numOfThreads >= MAXTHREADS) {ThreadYield();}
        ThreadCreate(Parking4);  // have available thread, create a new one for car 4

        while (numOfThreads >= MAXTHREADS) {ThreadYield();}
        ThreadCreate(Parking5);  // have available thread, create a new one for car 5
        
        while (numOfThreads != 1) {ThreadYield();}
        
        ThreadExit();  // all parking finished, exit this example
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