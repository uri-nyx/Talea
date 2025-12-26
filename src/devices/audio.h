#ifndef AUDIO_H
#define AUDIO_H

#include "emu2413.h"
#include "talea.h"

/*
 * Standard clock = MSX clock
 */
#define SYNTH_MSX_CLK 3579545

#define SYNTH_SAMPLERATE 44100
#define SYNTH_DATALENGTH (SAMPLERATE * 8)

void Synth_OPLL(void *, unsigned int);
#endif