#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"

static int delaysize = 200000; // includes both channels
static short delaybfr[MAX_DELAY_BUF_SIZE];

static short* feedbackptr = delaybfr+MAX_DELAY_BUF_SIZE-1 ;
static short* delayptr = delaybfr+MAX_DELAY_BUF_SIZE-delaysize-1; // bascially the current output

// Expects interleaved stereo 16-bit signed signal 
void delay_process(short* outputbfr, short* inputbfr, size_t numBytes) {
	short* inputptr = inputbfr;
	short* outputptr = outputbfr;

	for (unsigned int i = 0; i<numBytes/2; i++) {
		*delayptr = (*inputptr * 50 + *feedbackptr * 50) / 100;
		*outputptr = (*feedbackptr * 50 + *inputptr * 50) / 100;
		
		++outputptr;
		++inputptr;

		// shift delay buffer one sample
		if (delayptr == delaybfr) {
			delayptr = delaybfr+MAX_DELAY_BUF_SIZE-1;
		}
		else {
			--delayptr;
		}

		if (feedbackptr == delaybfr) {
			feedbackptr = delaybfr+MAX_DELAY_BUF_SIZE-1;
		}
		else {
			--feedbackptr;
		}
	}
}
