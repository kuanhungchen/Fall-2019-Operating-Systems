#
# makefile for testing two producers and one consumer
#
# This assumes you have SDCC installed and this targets EdSim51.
# The generated .hex file should be one that can be loaded and run
# directly.
#
# Author: Chen Kuan-Hung
# Student ID: 105061171
# Mail: khchen.me@gmail.com
# Course: Operating Systems (CS3423 Fall 2019)
#

CC = sdcc
CFLAGS = -c 
LDFLAGS = 
#--stack-after-data --stack-loc 0x39 --data-loc 0x20

C_OBJECTS = test3thread.rel preemptive.rel

all: test3thread.hex

test3thread.hex:   $(C_OBJECTS) $(ASM_OBJECTS)
				$(CC) $(LDFLAGS) -o test3thread.hex $(C_OBJECTS)

clean:
	rm *.hex *.asm *.lk *.lst *.map *.mem *.rel *.rst *.sym

%.rel:  %.c    preemptive.h Makefile
	$(CC) $(CFLAGS) $<
