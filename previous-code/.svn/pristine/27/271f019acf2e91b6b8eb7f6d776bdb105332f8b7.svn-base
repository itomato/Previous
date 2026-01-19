/*
  Previous - audio.c

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  This file contains the SDL interface for sound input and sound output.
*/
const char Audio_fileid[] = "Previous audio.c";

#include "main.h"
#include "statusbar.h"
#include "configuration.h"
#include "m68000.h"
#include "audio.h"
#include "dma.h"
#include "snd.h"
#include "host.h"
#include "grab.h"

#include <SDL3/SDL.h>


static SDL_AudioStream* Audio_Input_Stream  = NULL;
static SDL_AudioStream* Audio_Output_Stream = NULL;

static bool bSoundOutputWorking = false; /* Is sound output OK */
static bool bSoundInputWorking  = false; /* Is sound input OK */
static bool bPlayingBuffer      = false; /* Is playing buffer? */
static bool bRecordingBuffer    = false; /* Is recording buffer? */


/*-----------------------------------------------------------------------*/
/**
 * Sound output functions.
 */
void Audio_Output_Queue_Put(uint8_t* data, int len) {
	if (len > 0) {
		Grab_Sound(data, len);
		if (bSoundOutputWorking) {
			SDL_PutAudioStreamData(Audio_Output_Stream, data, len);
		}
	}
}

int Audio_Output_Queue_Size(void) {
	if (bSoundOutputWorking) {
		return SDL_GetAudioStreamQueued(Audio_Output_Stream) / 4;
	} else {
		return 0;
	}
}

void Audio_Output_Queue_Flush(void) {
	if (bSoundOutputWorking) {
		SDL_FlushAudioStream(Audio_Output_Stream);
	}
}

void Audio_Output_Queue_Clear(void) {
	if (bSoundOutputWorking) {
		SDL_ClearAudioStream(Audio_Output_Stream);
	}
}

/*-----------------------------------------------------------------------*/
/**
 * Sound input functions.
 *
 * Initialize recording buffer with silence to compensate for time gap
 * between Audio_Input_Enable and first availability of recorded data.
 */
#define AUDIO_RECBUF_INIT 32 /* 16000 byte = 1 second */

static uint8_t  recBuffer[1024];
static int recBufferReadPtr = 0;
static int recBufferSize    = 0;

static void Audio_Input_InitBuf(void) {
	Log_Printf(LOG_WARN, "[Audio] Initializing input buffer with %d ms of silence.", AUDIO_RECBUF_INIT>>4);
	recBufferReadPtr = 0;
	for (recBufferSize = 0; recBufferSize < AUDIO_RECBUF_INIT; recBufferSize++) {
		recBuffer[recBufferSize] = 0;
	}
}

int Audio_Input_Buffer_Size(void) {
	if (bSoundInputWorking) {
		return SDL_GetAudioStreamAvailable(Audio_Input_Stream);
	}
	return 0;
}

int Audio_Input_Buffer_Get(int16_t* sample) {
	if (bSoundInputWorking) {
		if (recBufferReadPtr >= recBufferSize) { /* Try to re-fill buffer in case it is empty. */
			recBufferReadPtr = 0;
			recBufferSize    = SDL_GetAudioStreamData(Audio_Input_Stream, recBuffer, sizeof(recBuffer));
			if (recBufferSize&1) {
				Log_Printf(LOG_WARN, "[Audio] Recording buffer has invalid size (%d).", recBufferSize);
				recBufferSize--;
			}
		}
		if (recBufferReadPtr < recBufferSize) {
			*sample = ((recBuffer[recBufferReadPtr]<<8)|recBuffer[recBufferReadPtr+1]);
			recBufferReadPtr += 2;
		} else {
			return -1;
		}
	} else {
		*sample = 0; /* silence */
	}
	return 0;
}

/*-----------------------------------------------------------------------*/
/**
 * Initialize the audio subsystem. Return true if all OK.
 */
void Audio_Output_Init(void)
{
	SDL_AudioSpec request = {SDL_AUDIO_S16BE, 2, SOUND_OUT_FREQUENCY};

	bSoundOutputWorking = false;

	/* Init the SDL's audio subsystem: */
	if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
			Log_Printf(LOG_WARN, "[Audio] Could not init audio output: %s\n", SDL_GetError());
			Statusbar_AddMessage("Error: Can't open SDL audio subsystem.", 5000);
			return;
		}
	}

	if (Audio_Output_Stream == NULL) {
		Audio_Output_Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &request, NULL, NULL);
	}
	if (Audio_Output_Stream == NULL) {
		Log_Printf(LOG_WARN, "[Audio] Could not open audio output device: %s\n", SDL_GetError());
		Statusbar_AddMessage("Error: Can't open audio output device. No sound output.", 5000);
		return;
	}

	bSoundOutputWorking = true;
}

void Audio_Input_Init(void) {
	SDL_AudioSpec request = {SDL_AUDIO_S16BE, 1, SOUND_IN_FREQUENCY};

	bSoundInputWorking = false;

	/* Init the SDL's audio subsystem: */
	if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
			Log_Printf(LOG_WARN, "[Audio] Could not init audio input: %s\n", SDL_GetError());
			Statusbar_AddMessage("Error: Can't open SDL audio subsystem.", 5000);
			return;
		}
	}

	if (Audio_Input_Stream == NULL) {
		Audio_Input_Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &request, NULL, NULL);
	}
	if (Audio_Input_Stream == NULL) {
		Log_Printf(LOG_WARN, "[Audio] Could not open audio input device: %s\n", SDL_GetError());
		Statusbar_AddMessage("Error: Can't open audio input device. Recording silence.", 5000);
		return;
	}

	bSoundInputWorking = true;
}

/*-----------------------------------------------------------------------*/
/**
 * Free audio subsystem
 */
void Audio_Output_UnInit(void) {
	if (bSoundOutputWorking) {
		/* Stop */
		Audio_Output_Enable(false);

		SDL_DestroyAudioStream(Audio_Output_Stream);
		Audio_Output_Stream = NULL;

		bSoundOutputWorking = false;
	}
}

void Audio_Input_UnInit(void) {
	if (bSoundInputWorking) {
		/* Stop */
		Audio_Input_Enable(false);

		SDL_DestroyAudioStream(Audio_Input_Stream);
		Audio_Input_Stream = NULL;

		bSoundInputWorking = false;
	}
}

/*-----------------------------------------------------------------------*/
/**
 * Start/Stop sound buffer
 */
void Audio_Output_Enable(bool bEnable) {
	if (bEnable && !bPlayingBuffer) {
		/* Start playing */
		SDL_ResumeAudioStreamDevice(Audio_Output_Stream);
		bPlayingBuffer = true;
	}
	else if (!bEnable && bPlayingBuffer) {
		/* Stop from playing */
		SDL_PauseAudioStreamDevice(Audio_Output_Stream);
		bPlayingBuffer = false;
	}
}

void Audio_Input_Enable(bool bEnable) {
	if (bEnable && !bRecordingBuffer) {
		/* Start recording */
		Audio_Input_InitBuf();
		SDL_ResumeAudioStreamDevice(Audio_Input_Stream);
		bRecordingBuffer = true;
	}
	else if (!bEnable && bRecordingBuffer) {
		/* Stop recording */
		SDL_PauseAudioStreamDevice(Audio_Input_Stream);
		bRecordingBuffer = false;
	}
}
