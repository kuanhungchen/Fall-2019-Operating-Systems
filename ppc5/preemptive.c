#include <8051.h>
#include "preemptive.h"

__data __at (0x20) int oldSP;
__data __at (0x21) int createdThreadSP;
__data __at (0x22) ThreadID createdThreadID;

__data __at (0x23) char EnterTimes[5];
__data __at (0x28) char LeaveTimes[5];

__data __at (0x2D) char i;
__data __at (0x2E) int numOfThreads;

__data __at (0x30) int th0_SP;
__data __at (0x31) int th1_SP;
__data __at (0x32) int th2_SP;
__data __at (0x33) int th3_SP;
__data __at (0x34) int threadBitMap;
__data __at (0x35) ThreadID currentThreadID;

__data __at (0x36) char mutex;
__data __at (0x37) char spots[2];
__data __at (0x39) char delays[3];
__data __at (0x3C) char Time;
__data __at (0x3D) char thrd_4;
__data __at (0x3E) char thrd_5;

int char_to_int(char c) {return c - '0';}

void delay(unsigned char n) {
      /* set up a delay for current thread */
      EA = 0;
      switch (currentThreadID){
         case '1':
            delays[0] = n;
            break;
         case '2':
            delays[1] = n;
            break;
         case '3':
            delays[2] = n;
            break;
         default:
            break;
      }
      EA = 1;
}

unsigned char now(void) {
      /* return the current time stamp */
      return Time;
}

#define SAVESTATE { \
   /* macro for saving state */ \
   __asm \
      PUSH ACC \
      PUSH B \
      PUSH DPL \
      PUSH DPH \
      PUSH PSW \
   __endasm; \
   switch (currentThreadID) { \
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

#define RESTORESTATE { \
   /* macro for restoring state */ \
   switch (currentThreadID) { \
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

extern void main(void);

void Bootstrap(void) {
      /* initialize some parameters, and create a thread for main() */
      for (i=0; i<2; i++) {
         spots[i] = '_';
      }
      for (i=0; i<5; i++) {
         EnterTimes[i] = '_';
         LeaveTimes[i] = '_';
      }

      thrd_4 = 'x';
      thrd_5 = 'x';

      threadBitMap = 0x00;
      th0_SP = 0x3F;
      th1_SP = 0x4F;
      th2_SP = 0x5F;
      th3_SP = 0x6F;

      TMOD = 0;
      IE = 0x82;
      TR0 = 1;

      currentThreadID = ThreadCreate(main);
      Time = 'a' - 1;
      numOfThreads = 1;
      RESTORESTATE;
}

void myTimer0Handler(void) {
      /* handler for timer 0, switching to another appropriate thread */
      EA = 0;
      SAVESTATE;
      
      Time += 1;
      for (i=0; i<3; i++) {  // decrement delays
         if (delays[i] > '0') {delays[i] -= 1;}
      }      
      if (mutex == 0) {
         // no spots available, switch to a car to leave
         if ((LeaveTimes[0] == '_') && (delays[0] == '0')) currentThreadID = '1';
         else if ((LeaveTimes[1] == '_') && (delays[1] == '0')) currentThreadID = '2';
         else if ((LeaveTimes[2] == '_') && (delays[2] == '0')) currentThreadID = '3';
         else if ((thrd_4 != 'x') && (LeaveTimes[3] == '_') && (delays[char_to_int(thrd_4)] == '0')) currentThreadID = thrd_4 + 1;
         else if ((thrd_5 != 'x') && (LeaveTimes[4] == '_') && (delays[char_to_int(thrd_5)] == '0')) currentThreadID = thrd_5 + 1;
         else if (LeaveTimes[0] == '_') currentThreadID = '1';
         else if (LeaveTimes[1] == '_') currentThreadID = '2';
         else if (LeaveTimes[2] == '_') currentThreadID = '3';
         else if (LeaveTimes[3] == '_') currentThreadID = thrd_4 + 1;
         else if (LeaveTimes[4] == '_') currentThreadID = thrd_5 + 1;
      } else {
         // some spots available, switch to a car to enter
         if (EnterTimes[0] == '_') currentThreadID = '1';
         else if (EnterTimes[1] == '_') currentThreadID = '2';
         else if (EnterTimes[2] == '_') currentThreadID = '3';
         else if (EnterTimes[3] == '_') currentThreadID = thrd_4 + 1;
         else if (EnterTimes[4] == '_') currentThreadID = thrd_5 + 1;
         else if (LeaveTimes[0] == '_') currentThreadID = '1';
         else if (LeaveTimes[1] == '_') currentThreadID = '2';
         else if (LeaveTimes[2] == '_') currentThreadID = '3';
         else if (LeaveTimes[3] == '_') currentThreadID = thrd_4 + 1;
         else if (LeaveTimes[4] == '_') currentThreadID = thrd_5 + 1;
      }

      RESTORESTATE;
      EA = 1;

      __asm
         RETI
      __endasm;
}

ThreadID ThreadCreate(FunctionPtr fp) {
         /* create a new thread */
         EA = 0;

         if ((threadBitMap & 0xFF) == 0xFF) {  // maximum threads existed
            return -1;
         }

         numOfThreads ++;
         createdThreadID = 'x';
         if ((threadBitMap & 0x01) == 0x00) {
            __asm
               MOV 0x22, #48
               ORL 0x34, #01
               MOV 0x21, 0x30
            __endasm;
         } else if ((threadBitMap & 0x02) == 0x00) {
            __asm
               MOV 0x22, #49
               ORL 0x34, #02
               MOV 0x21, 0x31
            __endasm;
         } else if ((threadBitMap & 0x04) == 0x00) {
            __asm
               MOV 0x22, #50
               ORL 0x34, #04
               MOV 0x21, 0x32
            __endasm;
         } else if ((threadBitMap & 0x08) == 0x00) {
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
            PUSH ACC  // ACC
            PUSH ACC  // B
            PUSH ACC  // DPL
            PUSH ACC  // DPH
         __endasm;

         switch (createdThreadID) {
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
         return createdThreadID;
}

void ThreadYield(void) {
      /* actively switch to another appropriate thread, same as timer interrupt handler */
      EA = 0;
      SAVESTATE;

      Time += 1;      
      for (i=0; i<3; i++) {  // decrement delays
         if (delays[i] > '0') {delays[i] -= 1;}
      }      
      if (mutex == 0) {
         // no spots available, switch to a car to leave
         if ((LeaveTimes[0] == '_') && (delays[0] == '0')) currentThreadID = '1';
         else if ((LeaveTimes[1] == '_') && (delays[1] == '0')) currentThreadID = '2';
         else if ((LeaveTimes[2] == '_') && (delays[2] == '0')) currentThreadID = '3';
         else if ((thrd_4 != 'x') && (LeaveTimes[3] == '_') && (delays[char_to_int(thrd_4)] == '0')) currentThreadID = thrd_4 + 1;
         else if ((thrd_5 != 'x') && (LeaveTimes[4] == '_') && (delays[char_to_int(thrd_5)] == '0')) currentThreadID = thrd_5 + 1;
         else if (LeaveTimes[0] == '_') currentThreadID = '1';
         else if (LeaveTimes[1] == '_') currentThreadID = '2';
         else if (LeaveTimes[2] == '_') currentThreadID = '3';
         else if (LeaveTimes[3] == '_') currentThreadID = thrd_4 + 1;
         else if (LeaveTimes[4] == '_') currentThreadID = thrd_5 + 1;
      } else {
         // some spots available, switch to a car to enter
         if (EnterTimes[0] == '_') currentThreadID = '1';
         else if (EnterTimes[1] == '_') currentThreadID = '2';
         else if (EnterTimes[2] == '_') currentThreadID = '3';
         else if (EnterTimes[3] == '_') currentThreadID = thrd_4 + 1;
         else if (EnterTimes[4] == '_') currentThreadID = thrd_5 + 1;
         else if (LeaveTimes[0] == '_') currentThreadID = '1';
         else if (LeaveTimes[1] == '_') currentThreadID = '2';
         else if (LeaveTimes[2] == '_') currentThreadID = '3';
         else if (LeaveTimes[3] == '_') currentThreadID = thrd_4 + 1;
         else if (LeaveTimes[4] == '_') currentThreadID = thrd_5 + 1;
      }
      RESTORESTATE;
      EA = 1;
}

void PrintParkingResult();

void ThreadExit(void) {
      /* terminate a thread and recycle its resource */
      numOfThreads --;
      switch (currentThreadID) {
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
      }
      if (currentThreadID == '0') {
         // main component exits, print out parking information
         EA = 0;
         PrintParkingResult();
         while (1) {}  // infinite loop
      } else {
         // thread exits, switch to main thread to create a new one if needed
         if (thrd_4 == 'x') thrd_4 = (currentThreadID == '1') ? '0' : '1';
         else if (thrd_5 == 'x') {
            if (currentThreadID == '1') thrd_5 = '0';
            else if (currentThreadID == '2') thrd_5 = '1';
            else if (currentThreadID == '3') thrd_5 = '2';
         }
         currentThreadID = '0';
      }
      RESTORESTATE;
}

void PrintParkingResult() {
   /* print out result of parking lot example */

   // car 1 info
   while (!TI) {}
   SBUF = '1';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   while (!TI) {}
   SBUF = EnterTimes[0];
   TI = 0;
   while (!TI) {}
   SBUF = '~';
   TI = 0;
   while (!TI) {}
   SBUF = LeaveTimes[0] - 1;
   TI = 0;
   while (!TI) {}
   SBUF = '|';
   TI = 0;
   // car 2 info
   while (!TI) {}
   SBUF = '2';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   while (!TI) {}
   SBUF = EnterTimes[1] - 1;
   TI = 0;
   while (!TI) {}
   SBUF = '~';
   TI = 0;
   while (!TI) {}
   SBUF = LeaveTimes[1] - 2;
   TI = 0;
   while (!TI) {}
   SBUF = '|';
   TI = 0;
   // car 3 info
   while (!TI) {}
   SBUF = '3';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   if ((EnterTimes[2] - LeaveTimes[0]) == 1) {
      while (!TI) {}
      SBUF = EnterTimes[2] - 1;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = LeaveTimes[2] - 2;
      TI = 0;
   } else if ((EnterTimes[2] - LeaveTimes[1]) == 1) {
      while (!TI) {}
      SBUF = EnterTimes[2] - 2;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = LeaveTimes[2] - 3;
      TI = 0;
   }
   while (!TI) {}
   SBUF = '|';
   TI = 0;
   // car 4 info
   while (!TI) {}
   SBUF = '4';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   if ((EnterTimes[3] - LeaveTimes[0]) == 1) {
      while (!TI) {}
      SBUF = EnterTimes[3] - 1;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = LeaveTimes[3] - 2;
      TI = 0;
   } else if ((EnterTimes[3] - LeaveTimes[1]) == 1){
      while (!TI) {}
      SBUF = EnterTimes[3] - 2;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = LeaveTimes[3] - 3;
      TI = 0;
   } else if ((EnterTimes[3] - LeaveTimes[2]) == 1) {
      if ((EnterTimes[2] - LeaveTimes[0]) == 1) {
         while (!TI) {}
         SBUF = EnterTimes[3] - 2;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = LeaveTimes[3] - 3;
         TI = 0;
      } else if ((EnterTimes[2] - LeaveTimes[1]) == 1) {
         while (!TI) {}
         SBUF = EnterTimes[3] - 3;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = LeaveTimes[3] - 4;
         TI = 0;
      }
   }
   while (!TI) {}
   SBUF = '|';
   TI = 0;
   // car 5 info
   while (!TI) {}
   SBUF = '5';
   TI = 0;
   while (!TI) {}
   SBUF = ':';
   TI = 0;
   if ((EnterTimes[4] - LeaveTimes[0]) == 1) {
      while (!TI) {}
      SBUF = EnterTimes[4] - 1;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = LeaveTimes[4] - 2;
      TI = 0;
   } else if ((EnterTimes[4] - LeaveTimes[1]) == 1){
      while (!TI) {}
      SBUF = EnterTimes[4] - 2;
      TI = 0;
      while (!TI) {}
      SBUF = '~';
      TI = 0;
      while (!TI) {}
      SBUF = LeaveTimes[4] - 3;
      TI = 0;
   } else if ((EnterTimes[4] - LeaveTimes[2]) == 1) {
      if ((EnterTimes[2] - LeaveTimes[0]) == 1) {
         while (!TI) {}
         SBUF = EnterTimes[4] - 2;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = LeaveTimes[4] - 3;
         TI = 0;
      } else if ((EnterTimes[2] - LeaveTimes[1]) == 1) {
         while (!TI) {}
         SBUF = EnterTimes[4] - 3;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = LeaveTimes[4] - 4;
         TI = 0;
      }
   } else if ((EnterTimes[4] - LeaveTimes[3]) == 1) {
      if ((EnterTimes[3] - LeaveTimes[0]) == 1) {
         while (!TI) {}
         SBUF = EnterTimes[4] - 2;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = LeaveTimes[4] - 3;
         TI = 0;
      } else if ((EnterTimes[3] - LeaveTimes[1]) == 1) {
         while (!TI) {}
         SBUF = EnterTimes[4] - 3;
         TI = 0;
         while (!TI) {}
         SBUF = '~';
         TI = 0;
         while (!TI) {}
         SBUF = LeaveTimes[4] - 4;
         TI = 0;
      } else if ((EnterTimes[3] - LeaveTimes[2]) == 1) {
         if ((EnterTimes[2] - LeaveTimes[0]) == 1) {
            while (!TI) {}
            SBUF = EnterTimes[4] - 3;
            TI = 0;
            while (!TI) {}
            SBUF = '~';
            TI = 0;
            while (!TI) {}
            SBUF = LeaveTimes[4] - 4;
            TI = 0;
         } else if ((EnterTimes[2] - LeaveTimes[1]) == 1){
            while (!TI) {}
            SBUF = EnterTimes[4] - 4;
            TI = 0;
            while (!TI) {}
            SBUF = '~';
            TI = 0;
            while (!TI) {}
            SBUF = LeaveTimes[4] - 5;
            TI = 0;
         }
      }
   }
   while (!TI) {}
   SBUF = '|';
   TI = 0;
}