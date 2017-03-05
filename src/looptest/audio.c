#ifdef WIN32
#include "SDL_config.h"
#else
#include <SDL2/SDL_config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

#include "audio.h"

struct
{
	SDL_AudioSpec spec;
	Uint8 *sound;               /* Pointer to wave data */
	Uint32 soundlen;            /* Length of wave data */
	int soundpos;               /* Current play position */
} wave;

static int done = 0;

#ifdef WIN32
static int buffersize = 16384;
static Uint8 delaybfr[16384 * 4];
#else
static int buffersize = 8192;
static Uint8 delaybfr[8192 * 4];
#endif


static void
quit(int rc)
{
	SDL_Quit();
	exit(rc);
}

void SDLCALL
fillerup(void *unused, Uint8 * stream, int len)
{
	SDL_Log("%i", len);

	Uint8 *waveptr;
	int waveleft;

	/* Set up the pointers */
	waveptr = wave.sound + wave.soundpos;
	waveleft = wave.soundlen - wave.soundpos;

	/* Go! */
	while (waveleft <= len) {
		SDL_memcpy(stream, waveptr, waveleft);
		stream += waveleft;
		len -= waveleft;
		waveptr = wave.sound;
		waveleft = wave.soundlen;
		wave.soundpos = 0;
	}
	SDL_memcpy(stream, waveptr, len);
	wave.soundpos += len;
}

void SDLCALL
fillerup_delay(void *unused, Uint8 * stream, int len)
{
	SDL_Log("%i", len);
	// curretly assumes buffersize bytes each request
	
	// shift everything down the buffer
	Uint8 *waveptr = delaybfr + buffersize;

	SDL_Log("copying...");
	memcpy(delaybfr, waveptr, buffersize*3);
	
	int waveleft;

	SDL_Log("setting...");

	/* Set up the pointers */
	waveptr = wave.sound + wave.soundpos;
	waveleft = wave.soundlen - wave.soundpos;

	// copy the new block to the end of the delay buffer
	Uint8* delayptr = delaybfr + buffersize*3;

	SDL_Log("sourcing...");

	/* Go! */
	while (waveleft <= len) {
		memcpy(delayptr, waveptr, waveleft);
		delayptr += waveleft;
		len -= waveleft;
		waveptr = wave.sound;
		waveleft = wave.soundlen;
		wave.soundpos = 0;
	}

	SDL_Log("final source...%i", len);

	memcpy(delayptr, waveptr, len);

	SDL_Log("updating soundpos...");
	wave.soundpos += len;

	SDL_Log("mixing...");

	// mix new signal with delayed signal
	Sint16* mixptr = (Sint16*)delaybfr;
	// Ptr to the 'live' signal
	Sint16* sourceptr = ((Sint16*)delaybfr) + buffersize / 2 *3;

	for(int i=0; i<buffersize/2; i++) {
		Sint32 delay = (*mixptr);
		Sint32 source = (*sourceptr);
		Sint32 mix =  ((delay*75+source*25)/100); // heavy dealy

		//mix = *sourceptr;
		*mixptr = (Sint32)mix;

		// add feedback
		*sourceptr = mix;

		//SDL_Log("%d,%d", mix, mix2);

		++mixptr;
		++sourceptr;
		
	}	
		
	SDL_memcpy(stream, delaybfr, buffersize);
}


int looptest_runloop() {
	int i;
	char filename[4096];

	/* Enable standard application logging */
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

	/* Load the SDL library */
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}

	// 8 bit sample
	//SDL_strlcpy(filename, "sample.wav", sizeof(filename));

	// 16 bit sample
	SDL_strlcpy(filename, "777sample.wav", sizeof(filename));

	if (SDL_LoadWAV(filename, &wave.spec, &wave.sound, &wave.soundlen) == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s\n", filename, SDL_GetError());
		quit(1);
	}

	wave.spec.callback = fillerup_delay;
	//wave.spec.callback = fillerup;

	/* Show the list of available drivers */
	SDL_Log("Available audio drivers:");
	for (i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
		SDL_Log("%i: %s", i, SDL_GetAudioDriver(i));
	}

	/* Initialize fillerup() variables */
	if (SDL_OpenAudio(&wave.spec, NULL) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio: %s\n", SDL_GetError());
		SDL_FreeWAV(wave.sound);
		quit(2);
	}

	SDL_Log("Using audio driver: %s\n", SDL_GetCurrentAudioDriver());

	SDL_Log("\nSDL_AudioSpec:");
	SDL_Log("freq: %i", wave.spec.freq);
	SDL_Log("channels: %i", wave.spec.channels);
	SDL_Log("buffer size (samples): %i", wave.spec.samples);
	SDL_Log("buffer size (bytes): %i", wave.spec.size);
	SDL_Log("format.bitsize: %i", SDL_AUDIO_BITSIZE(wave.spec.format));
	SDL_Log("format.isfloat: %i", SDL_AUDIO_ISFLOAT(wave.spec.format));
	SDL_Log("format.isbigendian: %i", SDL_AUDIO_ISBIGENDIAN(wave.spec.format));
	SDL_Log("format.issigned: %i", SDL_AUDIO_ISSIGNED(wave.spec.format));

	SDL_Log("\n");

	SDL_PauseAudio(0);

	while (!done && (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING)) {
		SDL_Delay(1000);
	}

	/* Clean up on signal */
	SDL_CloseAudio();
	SDL_FreeWAV(wave.sound);
	SDL_Quit();

	return (0);
}
