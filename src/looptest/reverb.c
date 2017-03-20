#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reverb.h"


#define RESNUM 15 // Number of resonators; res 0 is input and 1 is output

static short outbfr[RESNUM][MAX_DELAY_BUF_SIZE]; // buffer for each resonator
static short* outptr[RESNUM]; // list of pointers for it's current output

static unsigned short sources[RESNUM][RESNUM-2]; // each resonator can listen to at most RESNUM - 2 sources (excludes itself and output); holds a RESNUM index
static unsigned int delays[RESNUM][RESNUM-2];  // each source has a delay value associated which is an offest from the source output
static short* sourceptr[RESNUM][RESNUM-2]; // list of pointers for each source


// Positions of all resonators
static int xpos[RESNUM];
static int ypos[RESNUM];

// Distance of primary ring
static unsigned int r = 100000; 

#define PI 3.1415

void init_positions() {
	// Initialize positons of resonators
	// Currently just an evently distributed ring of resonators with source and output at center
	// No stereo separation currently (likely will just separate L & R  channels by a fixed distance for both source and output

	xpos[0] = 0;
	ypos[0] = 0;
	xpos[1] = 0;
	xpos[1] = 0;

	// Calcualte resonators (excluding source and output, which are at origin)
	for (int i=2; i<RESNUM; i++) {
		double a = 2 * PI / i-2;
		xpos[i] = r * cos(a);
		ypos[i] = r * sin(a);
	}
}

void clear_mem() {
	for(int i=0; i<RESNUM; i++) {
		memset(outbfr[RESNUM], 0, MAX_DELAY_BUF_SIZE * sizeof(short));
		memset(sources[RESNUM], 0, (RESNUM-2) * sizeof(short));
		memset(delays[RESNUM], 0, (RESNUM-2) * sizeof(short));
	}
}

void init_sources_static() {
	// RESNUM0 is input; listens to no one
	// RESNUM1 is output; listens to everyone except input and self; has no listeners
	// RESNUM2+ is resonator; listens to everyone except self and output
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
}

void init_sources() {
	for (int i=1; i<RESNUM; i++) {
		unsigned short curref;
		if (i==1) {
			curref = 2;
		}
		else {
			curref = 0;
		}

		for (int j=0; j<RESNUM-2; j++) {
			if (curref==1) curref++;
			if (curref==i) curref++;
			sources[i][j] = curref++;
		}
	}
}

void init_delays_static() {
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
}

unsigned int calc_delay(short r1, short r2) {
	// calculate delay based on distance between res1 and res2

	int x1 = xpos[r1];
	int y1 = ypos[r1];
	int x2 = xpos[r2];
	int y2 = ypos[r2];

	//printf("%i,%i:%i,%i;%i,%i\n", r1, r2, x1, y1, x2, y2);

	long xx = x2-x1;
	long yy = y2-y1;

	double h = sqrt(xx*xx + yy*yy);

	double delayScale = 1;
	return h * delayScale;
}

void init_delays() {
	// Set delays based on calcualted distance between resonators, including source and output

	// Exclude input from source map
	for(int i=1; i<RESNUM; i++) {
		for (int j=0; j<RESNUM-2; j++) {
			unsigned int delay = calc_delay(i, sources[i][j]);
			delays[i][j] = delay;
			//printf("delays[%i,%i]: %i\n", i, j, delays[i][j]);
		}
	}
}

void reverb_init() {
	clear_mem();
	init_positions();

	init_sources();
	init_delays();

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
			long delayTotal = 0;
			for (int j = 0; j < RESNUM-2; j++) {
				val += *(sourceptr[i][j]);
				//val += *(sourceptr[i][j]) * (2 * r / delays[i][j]);
				//delayTotal += 2 * r / delays[i][j];
			}
			*(outptr[i]) = (short)(val/(RESNUM-2)); // even mix for now
			//*(outptr[i]) = (short)(val/delayTotal); // weighted mix by inverse distance
		}


		// Copy reverb output and mix live signal
		//*outputptr = (*(outptr[1]) * 90 + *inputptr * 10) / 100; 
		
		// Just wet signal
		*outputptr = *(outptr[1]);

		// Advance all pointers
		++outputptr;
		++inputptr;


		for (int i = 0; i < RESNUM; i++) {
			if (outptr[i] == outbfr[i]) {
				//printf("reset outptr[%i]\n", i);
				outptr[i] = outbfr[i] + MAX_DELAY_BUF_SIZE;
			}
			--(outptr[i]);
			
			// Exclude input from sourceptr processing
			if (i == 0) continue;

			for (int j = 0; j < RESNUM-2; j++) {
				if (sourceptr[i][j] == outbfr[sources[i][j]]) {
					//printf("reset srcptr[%i,%i]\n", i, j);
					sourceptr[i][j] = outbfr[sources[i][j]] + MAX_DELAY_BUF_SIZE;
				}
				--(sourceptr[i][j]);
			}
		}
	}
}
