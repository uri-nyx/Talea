#ifndef AUDIO_H
#define AUDIO_H


#include "types.h"
#include "logging.h"
#include "emu2413.h"

#ifndef DO_NOT_INCLUDE_RAYLIB
// Due to confilcts with the WinAPI
#include "raylib.h"
#else
// definitions we need from raylib
#include "need_from_raylib.h"
#endif

struct TaleaMachine;

/*----------------------------------------------------------------------------*/
/* DEVICE MAGIC ID                                                            */
/*----------------------------------------------------------------------------*/

#define DEVICE_AUDIO_MAGIC (1096107056) // 'AUD0'

/*----------------------------------------------------------------------------*/
/* DEVICE PORTS                                                               */
/*----------------------------------------------------------------------------*/

#define P_AUDIO_ADDR           0x0 // OPLL synth ADDR register
#define P_AUDIO_DATA           0x1 // OPLL synth DATA register
#define P_AUDIO_CSR            0x2 // AUDIO device CSR
#define P_AUDIO_FNUMH          0x3 // OPLL synth F num register, high byte
#define P_AUDIO_FNUML          0x4 // OPLL synth F num register, low byte
#define P_AUDIO_DURH           0x5 // Note duration register, high byte, in ms
#define P_AUDIO_DURL           0x6 // Note duration register, low byte, in ms
#define P_AUDIO_CHANNEL_SELECT 0x7 // OPLL channel select register, range 0..8
#define P_AUDIO_GLOBAL_STATUS0 0x8 // AUDIO device global status, high byte
#define P_AUDIO_GLOBAL_STATUS1 0x9 // AUDIO device global status, mid high byte
#define P_AUDIO_GLOBAL_STATUS2 0xa // AUDIO device global status, mid low byte
#define P_AUDIO_GLOBAL_STATUS3 0xb // AUDIO device global status, low byte
#define P_AUDIO_MASTER_VOL     0xc // AUDIO device global master volume register
#define P_AUDIO_PCM_FIFOH      0xd // PCM output queue register, high byte
#define P_AUDIO_PCM_FIFOL      0xe // PCM output queue register, low byte

/*----------------------------------------------------------------------------*/
/* DEVICE CONSTANTS                                                           */
/* ---------------------------------------------------------------------------*/

// CONSTANTS FOR THE OPLL SYNTH

#define AUDIO_OPLL_MSX_CLK      3579545
#define AUDIO_OPLL_SAMPLERATE   44100
#define AUDIO_OPLL_DATALENGTH   (SAMPLERATE * 8)
#define AUDIO_OPLL_NUM_CHANNELS 9

// COONSTANTS FOR THE PCM SAMPLE QUEUE

#define AUDIO_PCM_FIFO_SIZE 4096

/**
 * The AUDIO GLOBAL STATUS register is a 32 bit bitmap that reports the status
 * of all the channels of the OPLL synth at the same time. It also reports the
 * status of the PCM sample queue.
 */
enum AudioGlobalStatus {
    AUDIO_GLOB_NOTE_ENDED0       = (1U << 0U), // Note in channel 0 has ended
    AUDIO_GLOB_NOTE_ENDED1       = (1U << 1U), // Note in channel 1 has ended
    AUDIO_GLOB_NOTE_ENDED2       = (1U << 2U), // Note in channel 2 has ended
    AUDIO_GLOB_NOTE_ENDED3       = (1U << 3U), // Note in channel 3 has ended
    AUDIO_GLOB_NOTE_ENDED4       = (1U << 4U), // Note in channel 4 has ended
    AUDIO_GLOB_NOTE_ENDED5       = (1U << 5U), // Note in channel 5 has ended
    AUDIO_GLOB_NOTE_ENDED6       = (1U << 6U), // Note in channel 6 has ended
    AUDIO_GLOB_NOTE_ENDED7       = (1U << 7U), // Note in channel 7 has ended
    AUDIO_GLOB_NOTE_ENDED8       = (1U << 8U), // Note in channel 8 has ended
    AUDIO_GLOB_NOTE_ENDED_MASK   = 0x1ff,
    AUDIO_GLOB_BUSY0             = (1U << 9U),  // Channel 0 is playing a note
    AUDIO_GLOB_BUSY1             = (1U << 10U), // Channel 1 is playing a note
    AUDIO_GLOB_BUSY2             = (1U << 11U), // Channel 2 is playing a note
    AUDIO_GLOB_BUSY3             = (1U << 12U), // Channel 3 is playing a note
    AUDIO_GLOB_BUSY4             = (1U << 13U), // Channel 4 is playing a note
    AUDIO_GLOB_BUSY5             = (1U << 14U), // Channel 5 is playing a note
    AUDIO_GLOB_BUSY6             = (1U << 15U), // Channel 6 is playing a note
    AUDIO_GLOB_BUSY7             = (1U << 16U), // Channel 7 is playing a note
    AUDIO_GLOB_BUSY8             = (1U << 17U), // Channel 8 is playing a note
    AUDIO_GLOB_BUSY_MASK         = 0x3fE00,
    AUDIO_GLOB_PCM_FIFO_FULL     = (1U << 20U), // The PCM queue is full
    AUDIO_GLOB_PCM_LOW_WATERMARK = (1U << 21U), // The PCM queue is half-empty
};

/**
 * AUDIO device CONTROL STATUS REGISTER. Each OPLL channel has its own instance.
 */
enum AudioCsr {
    AUDIO_CSR_TRIGGER    = 1 << 0, // Starts a note with the parameters selected
    AUDIO_CSR_IE         = 1 << 1, // Enables interrupts on note end
    AUDIO_CSR_LOOP       = 1 << 2, // Loops note with current parameters
    AUDIO_CSR_BUSY       = 1 << 3, // Mirror of GLOBAL STATUS for this channel
    AUDIO_CSR_STOP       = 1 << 4, // Stops the channel inmediately
    AUDIO_CSR_NOTE_ENDED = 1 << 5, // Mirror of GLOBAL STATUS for this channel
    AUDIO_CSR_GATE       = 1 << 6, // OPLL Gate mode enable
};

/*----------------------------------------------------------------------------*/
/* DEVICE STRUCTURES                                                          */
/*--------------------------------------------------------------------------- */

/**
 * State of a channel in the OPLL synth
 */
typedef struct {
    u8   csr;  // CONTROL STATUS REGISTER, see AudioCsr
    u16  fnum; // Current F number of the note
    u16  dur;  // Current duration of the note, in ms
    u32  totalSamples;
    i64  samplesLeft;
    u8   lastFreqHigh;
    u8   lastFreqLow;
    bool sequencerActive;
} AudioOPLLChannel;

/**
 * PCM sample queue structure
 */
typedef struct {
    i16    buffer[AUDIO_PCM_FIFO_SIZE];
    size_t head;
    size_t tail;
} AudioPCMFifo;

/*----------------------------------------------------------------------------*/
/* DEVICE State                                                           */
/*----------------------------------------------------------------------------*/

typedef struct DeviceAudio {
    // OPLL synth state
    OPLL *opll;
    u8    selectedChannel;

    AudioOPLLChannel channels[AUDIO_OPLL_NUM_CHANNELS];

    // PCM sample queue
    AudioPCMFifo pcmFifo;
    u16          pcmSampleLatch;

    // Global AUDIO device state
    u32  globalStatus;
    u8   masterVolume;
    bool fireInterrupt;
} DeviceAudio;

/*----------------------------------------------------------------------------*/
/* DEVICE INTERFACE                                                           */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream
void Audio_OPLL(void *buffer, unsigned int frames);
void Audio_PCM(void *buffer, unsigned int frames);

// DEVICE INITIALIZATION
void Audio_Reset(struct TaleaMachine *m, bool isRestart);

// DEVICE UPDATE (on vblank)
void Audio_Update(struct TaleaMachine *m);

// DEVICE UPDATE (on cpu tick)
// NONE

// DEVICE PORT IO HANDLERS
void Audio_Write(struct TaleaMachine *m, u8 port, u8 value);
u8   Audio_Read(struct TaleaMachine *m, u8 port);

#endif