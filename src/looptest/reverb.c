#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reverb.h"


#define RESNUM 5 // Number of resonators; res 0 is input and 1 is output

static short outbfr[RESNUM][MAX_DELAY_BUF_SIZE]; // buffer for each resonator
static short* outptr[RESNUM]; // list of pointers for it's current output

static short sources[RESNUM][RESNUM-2]; // each resonator can listen to at most RESNUM - 2 sources (excludes itself and output); holds a RESNUM index
static short delays[RESNUM][RESNUM-2];  // each source has a delay value associated which is an offest from the source output
static short* sourceptr[RESNUM][RESNUM-2]; // list of pointers for each source


void reverb_init() {
	for(int i=0; i<RESNUM; i++) {
		memset(outbfr[RESNUM], 0, MAX_DELAY_BUF_SIZE * sizeof(short));
		memset(sources[RESNUM], 0, (RESNUM-1) * sizeof(short));
		memset(delays[RESNUM], 0, (RESNUM-1) * sizeof(short));
	}

	// RESNUM0 is input; listens to no one
	// RESNUM1 is output; listens to everyone except input and self; has no listeners
	// RESNUM2+ is resonator; listens to everyone except self;
	sources[1][0] = 2;
	sources[1][1] = 3;
	sources[1][2] = 4;

	sources[2][0] = 0;
	sources[2][1] = 3;
	sources[2][2] = 4;

	sources[3][0] = 0;
	sources[3][1] = 2;
	sources[3][2] = 4;

	sources[4][0] = 0;
	sources[4][1] = 2;
	sources[4][2] = 3;

	delays[1][0] = 30000;
	delays[1][1] = 50000;
	delays[1][2] = 2900;

	delays[2][0] = 32000;
	delays[2][1] = 3000;
	delays[2][2] = 42000;

	delays[3][0] = 3400;
	delays[3][1] = 43000;
	delays[3][2] = 49930;

	delays[4][0] = 3600;
	delays[4][1] = 37000;
	delays[4][2] = 3200;

	// Set initial outptrs
	for (int i = 0; i < RESNUM; i++) {
		outptr[i] = outbfr[i];
	}

	// Set initial sourceptrs excluding input
	for (int i = 1; i < RESNUM; i++) {
		for (int j = 0; j < RESNUM-2; j++) {
			sourceptr[i][j] = outbfr[sources[i][j]] + delays[i][j];
		}
	}
}

// Expects interleaved stereo 16-bit signed signal 
void reverb_process(short* outputbfr, short* inputbfr, size_t numBytes) {
	short* inputptr = inputbfr;
	short* outputptr = outputbfr;

	for (unsigned int cursample = 0; cursample<numBytes/2; cursample++) {
		// write input
		*(outptr[0]) = *inputptr;

		
		// Process all resonators including output
		for (int i = 1; i < RESNUM; i++) {
			long val = 0;
			for (int j = 0; j < RESNUM-2; j++) {
				val += *(sourceptr[i][j]);
			}
			*(outptr[i]) = (short)(val/5); // even mix for now
		}


		// Copy reverb output and mix live signal
		*outputptr = (*(outptr[1]) * 90 + *inputptr * 10) / 100; 


		// Advance all pointers
		++outputptr;
		++inputptr;


		for (int i = 0; i < RESNUM; i++) {
			if (outptr[i] == outbfr[i]) {
				outptr[i] = outbfr[i] + MAX_DELAY_BUF_SIZE;
			}
			--(outptr[i]);
			
			// Exclude input from sourceptr processing
			if (i == 0) continue;

			for (int j = 0; j < RESNUM-2; j++) {
				if (sourceptr[i][j] == outbfr[sources[i][j]]) {
					sourceptr[i][j] = outbfr[sources[i][j]] + MAX_DELAY_BUF_SIZE;
				}
				--(sourceptr[i][j]);
			}
		}
	}
}
