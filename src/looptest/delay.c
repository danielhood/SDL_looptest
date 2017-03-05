#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"

static int delaysize = 10000; // includes both chanels
static signed __int16 delaybfr[MAX_DELAY_BUF_SIZE];

// Expects interleaved stereo 16-bit signed signal 
void delay_process(signed __int16* outputbfr, signed __int16* inputbfr, size_t numBytes) {
	// simply copy input to output
	//memcpy(outputbfr, inputbfr, numBytes);
	//return; 

	signed __int16* inputptr = inputbfr;
	signed __int16* delayptr = delaybfr+delaysize-1;
	signed __int16* outputptr = outputbfr;
	signed __int16* feedbackptr = delaybfr;

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