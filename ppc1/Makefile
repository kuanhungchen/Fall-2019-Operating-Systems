#
# makefile for testing cooperative multithreading
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

C_OBJECTS = testcoop.rel cooperative.rel

all: testcoop.hex

testcoop.hex:   $(C_OBJECTS) $(ASM_OBJECTS)
				$(CC) $(LDFLAGS) -o testcoop.hex $(C_OBJECTS)

clean:
	rm *.hex *.asm *.lk *.lst *.map *.mem *.rel *.rst *.sym

%.rel:  %.c    cooperative.h Makefile
	$(CC) $(CFLAGS) $<
