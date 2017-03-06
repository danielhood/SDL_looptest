#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"

static int delaysize = 40000; // includes both chanels
static short delaybfr[MAX_DELAY_BUF_SIZE];

// Expects interleaved stereo 16-bit signed signal 
void delay_process(short* outputbfr, short* inputbfr, size_t numBytes) {
	// simply copy input to output
	//memcpy(outputbfr, inputbfr, numBytes);
	//return; 

	short* inputptr = inputbfr;
	short* delayptr = delaybfr+delaysize-1;
	short* outputptr = outputbfr;
	short* feedbackptr = delaybfr;

	for (unsigned int i = 0; i<numBytes/2; i++) {
		*delayptr = (*inputptr * 50 + *feedbackptr * 50) / 100;
		*outputptr = (*feedbackptr * 50 + *inputptr * 50) / 100;
		
		outputptr++;
		inputptr++;

		// shift delay buffer one sample
		// Need a circular buffer as the memcopy is silly
		memcpy(delaybfr, delaybfr + 1, (delaysize - 1) * 2);
	}
}
