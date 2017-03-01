#include "SDL_config.h"

#include <stdio.h>
#include <stdlib.h>


#include "SDL.h"

#include "audio.h"

struct
{
	SDL_AudioSpec spec;
	Uint8 *sound;               /* Pointer to wave data */
	Uint32 soundlen;            /* Length of wave data */
	int soundpos;               /* Current play position */
} wave;

static int done = 0;

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

	SDL_strlcpy(filename, "sample.wav", sizeof(filename));

	if (SDL_LoadWAV(filename, &wave.spec, &wave.sound, &wave.soundlen) == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s\n", filename, SDL_GetError());
		quit(1);
	}

	wave.spec.callback = fillerup;

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
	SDL_Log("samples: %i", wave.spec.samples);
	SDL_Log("buffer size: %i", wave.spec.size);
	SDL_Log("format.bitsize: %i", SDL_AUDIO_BITSIZE(wave.spec.format));
	SDL_Log("format.isfloat: %i", SDL_AUDIO_ISFLOAT(wave.spec.format));
	SDL_Log("format.isbigendian: %i", SDL_AUDIO_ISBIGENDIAN(wave.spec.format));
	SDL_Log("format.issigned: %i", SDL_AUDIO_ISSIGNED(wave.spec.format));

	SDL_Log("\n");

	//SDL_PauseAudio(0);

	//while (!done && (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING)) {
	//	SDL_Delay(1000);
	//}

	/* Clean up on signal */
	SDL_CloseAudio();
	SDL_FreeWAV(wave.sound);
	SDL_Quit();

	return (0);
}