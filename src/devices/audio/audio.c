#include "audio.h"
#include "core/bus.h"
#include "talea.h"

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

// ugly hack to be able to pass the synth to the callback
static DeviceAudio *Audio = NULL;

static const u8 volumeLut[] = { 0, 2, 4, 8, 16, 24, 32, 48, 64, 80, 96, 128, 160, 192, 224, 255 };

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/

static inline void clearBusy(DeviceAudio *a, u8 ch)
{
    a->channels[ch].csr &= ~AUDIO_CSR_BUSY;
    a->globalStatus &= ~(1 << (ch + AUDIO_OPLL_NUM_CHANNELS));
}

static inline void setBusy(DeviceAudio *a, u8 ch)
{
    a->channels[ch].csr |= AUDIO_CSR_BUSY;
    a->globalStatus |= (1 << (ch + AUDIO_OPLL_NUM_CHANNELS));
}

static inline void clearNoteEnded(DeviceAudio *a, u8 ch)
{
    a->channels[ch].csr &= ~AUDIO_CSR_NOTE_ENDED;
    a->globalStatus &= ~(1 << ch);
}

static inline void setNoteEnded(DeviceAudio *a, u8 ch)
{
    a->channels[ch].csr |= AUDIO_CSR_NOTE_ENDED;
    a->globalStatus |= (1 << ch);
}

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream

void Audio_OPLL(void *buffer, unsigned int frames)
{
    if (!Audio || !Audio->opll) return;

    i16 *wave = (i16 *)buffer;

    for (size_t i = 0; i < AUDIO_OPLL_NUM_CHANNELS; i++) {
        AudioOPLLChannel *chan = &Audio->channels[i];

        if (chan->sequencerActive) {
            chan->samplesLeft -= frames;

            if (chan->samplesLeft <= 0) {
                OPLL_writeReg(Audio->opll, 0x20 + i, chan->lastFreqHigh & 0xEF);

                if (chan->csr & AUDIO_CSR_LOOP) {
                    chan->samplesLeft = chan->totalSamples;
                    OPLL_writeReg(Audio->opll, 0x20 + i, 0x10 | chan->lastFreqHigh);

                    if (chan->csr & AUDIO_CSR_GATE) {
                        chan->samplesLeft = 0;
                        OPLL_writeReg(Audio->opll, 0x20 + i, chan->lastFreqHigh & 0xEF);
                    }
                } else {
                    chan->sequencerActive = false;
                    clearBusy(Audio, i);
                    setNoteEnded(Audio, i);

                    if (chan->csr & AUDIO_CSR_IE) {
                        Audio->fireInterrupt = true;
                    }
                }
            }
        }
    }

    i32 mult = volumeLut[Audio->masterVolume & 0x0F];
    for (size_t i = 0; i < frames; i++) {
        i16 sample = OPLL_calc(Audio->opll);
        i32 mixed  = ((i32)sample * mult) >> 8;
        mixed      = mixed > INT16_MAX ? INT16_MAX : mixed < INT16_MIN ? INT16_MIN : mixed;
        wave[i]    = (i16)mixed;
    }
}

void Audio_PCM(void *buffer, unsigned int frames)
{
    if (!Audio) return;

    i16 *wave = (i16 *)buffer;
    i32  mult = volumeLut[Audio->masterVolume & 0x0F];
    u16  pcmCount;

    if (Audio->pcmFifo.head >= Audio->pcmFifo.tail) {
        pcmCount = Audio->pcmFifo.head - Audio->pcmFifo.tail;
    } else {
        pcmCount = (AUDIO_PCM_FIFO_SIZE - Audio->pcmFifo.tail) + Audio->pcmFifo.head;
    }

    for (size_t i = 0; i < frames; i++) {
        if (Audio->pcmFifo.head != Audio->pcmFifo.tail) {
            i16 sample = Audio->pcmFifo.buffer[Audio->pcmFifo.tail];
            i32 mixed  = ((i32)sample * mult) >> 8;
            mixed      = mixed > INT16_MAX ? INT16_MAX : mixed < INT16_MIN ? INT16_MIN : mixed;
            wave[i]    = (i16)mixed;
            Audio->pcmFifo.tail = (Audio->pcmFifo.tail + 1) % AUDIO_PCM_FIFO_SIZE;
            Audio->globalStatus &= ~AUDIO_GLOB_PCM_FIFO_FULL;

            pcmCount--;

            if (pcmCount < (AUDIO_PCM_FIFO_SIZE / 2))
                Audio->globalStatus |= AUDIO_GLOB_PCM_LOW_WATERMARK;
        } else {
            wave[i] = 0; // Silence if FIFO is empty
        }
    }
}

// DEVICE INITIALIZATION

void Audio_Reset(TaleaMachine *m, bool isRestart)
{
    // maybe this is all over the place, but the frontend takes care of the
    // audio stream intializations. Here se simply assign the static pointer
    // to be a reference to the machine's DeviceAudio.

    Audio = &m->audio;
}

// DEVICE UPDATE (on vblank)
void Audio_Update(TaleaMachine *m)
{
    if (m->audio.fireInterrupt) {
        m->audio.fireInterrupt = false;
        Machine_RaiseInterrupt(m, INT_AUDIO_NOTE_END, PRIORITY_AUDIO_INTERRUPT);
    }
}

// DEVICE UPDATE (on cpu tick)
// NONE

// DEVICE PORT IO HANDLERS

u8 Audio_Read(TaleaMachine *m, u8 port)
{
    DeviceAudio *a  = &m->audio;
    u8           ch = a->selectedChannel % AUDIO_OPLL_NUM_CHANNELS;

    u16 pcmCount;
    if (a->pcmFifo.head >= a->pcmFifo.tail) {
        pcmCount = a->pcmFifo.head - a->pcmFifo.tail;
    } else {
        pcmCount = (AUDIO_PCM_FIFO_SIZE - a->pcmFifo.tail) + a->pcmFifo.head;
    }

    switch (port & 0xf) {
    case P_AUDIO_ADDR: return 0;
    case P_AUDIO_DATA: return 0;
    case P_AUDIO_CSR: return a->channels[ch].csr;
    case P_AUDIO_FNUMH: return a->channels[ch].fnum >> 8;
    case P_AUDIO_FNUML: return a->channels[ch].fnum;
    case P_AUDIO_DURH: return a->channels[ch].dur >> 8;
    case P_AUDIO_DURL: return a->channels[ch].dur;
    case P_AUDIO_CHANNEL_SELECT: return ch;
    case P_AUDIO_GLOBAL_STATUS0: return 0; break; // unused
    case P_AUDIO_GLOBAL_STATUS1: return a->globalStatus >> 16; break;
    case P_AUDIO_GLOBAL_STATUS2: return a->globalStatus >> 8; break;
    case P_AUDIO_GLOBAL_STATUS3: return a->globalStatus; break;
    case P_AUDIO_MASTER_VOL: return a->masterVolume; break;
    case P_AUDIO_PCM_FIFOH: return pcmCount >> 8; break;
    case P_AUDIO_PCM_FIFOL: return pcmCount; break;
    default: return 0;
    }
}

void Audio_Write(TaleaMachine *m, u8 port, u8 value)
{
    DeviceAudio      *a     = &m->audio;
    AudioOPLLChannel *chan  = &a->channels[a->selectedChannel % AUDIO_OPLL_NUM_CHANNELS];
    u8                ch_id = a->selectedChannel % AUDIO_OPLL_NUM_CHANNELS;
    u8                reg   = port & 0xf;

    switch (reg) {
    case P_AUDIO_ADDR:
    case P_AUDIO_DATA: OPLL_writeIO(a->opll, reg, value); break;
    case P_AUDIO_CSR:
        if (value & AUDIO_CSR_TRIGGER && !(chan->csr & AUDIO_CSR_BUSY)) {
            value &= ~AUDIO_CSR_TRIGGER;
            value &= ~AUDIO_CSR_NOTE_ENDED;
            clearNoteEnded(a, ch_id);
            value |= AUDIO_CSR_BUSY;
            setBusy(a, ch_id);

            chan->sequencerActive = true;

            chan->totalSamples = (chan->dur * AUDIO_OPLL_SAMPLERATE) / 1000;
            chan->samplesLeft  = chan->totalSamples;

            chan->lastFreqLow  = chan->fnum & 0xFF;
            chan->lastFreqHigh = chan->fnum >> 8;

            OPLL_writeReg(a->opll, 0x10 + ch_id, chan->lastFreqLow);
            OPLL_writeReg(a->opll, 0x20 + ch_id, 0x10 | chan->lastFreqHigh);
            if (value & AUDIO_CSR_GATE) {
                chan->samplesLeft = 0;
                OPLL_writeReg(a->opll, 0x10 + ch_id, chan->lastFreqLow);
                OPLL_writeReg(a->opll, 0x20 + ch_id, 0xef & chan->lastFreqHigh);
            }
        }

        if (value & AUDIO_CSR_STOP) {
            clearBusy(a, ch_id);
            value &= ~AUDIO_CSR_BUSY;
            value &= ~AUDIO_CSR_STOP;
            chan->sequencerActive = false;
            OPLL_writeReg(a->opll, 0x20 + ch_id, chan->lastFreqHigh & 0xEF);
        }

        chan->csr = value;

        break;
    case P_AUDIO_FNUMH: chan->fnum = (chan->fnum & 0x00FF) | (u16)value << 8; break;
    case P_AUDIO_FNUML: chan->fnum = (chan->fnum & 0xFF00) | value; break;
    case P_AUDIO_DURH: chan->dur = (chan->dur & 0x00FF) | (u16)value << 8; break;
    case P_AUDIO_DURL: chan->dur = (chan->dur & 0xFF00) | value; break;
    case P_AUDIO_CHANNEL_SELECT: a->selectedChannel = value % AUDIO_OPLL_NUM_CHANNELS; break;
    case P_AUDIO_GLOBAL_STATUS0: break;
    case P_AUDIO_GLOBAL_STATUS1:
        a->globalStatus = (a->globalStatus & 0x00ffff) | (u32)value << 16;
        break;
    case P_AUDIO_GLOBAL_STATUS2:
        a->globalStatus = (a->globalStatus & 0xff00ff) | (u32)value << 8;
        break;
    case P_AUDIO_GLOBAL_STATUS3: a->globalStatus = (a->globalStatus & 0xffff00) | (u32)value; break;
    case P_AUDIO_MASTER_VOL: a->masterVolume = value; break;
    case P_AUDIO_PCM_FIFOH: a->pcmSampleLatch = (u16)value << 8; break;
    case P_AUDIO_PCM_FIFOL: {
        // Only when we write to here, we push
        AudioPCMFifo *fifo = &a->pcmFifo;

        a->pcmSampleLatch |= (value & 0xff);

        size_t next_head = (fifo->head + 1) % AUDIO_PCM_FIFO_SIZE;
        if (next_head != fifo->tail) {
            fifo->buffer[fifo->head] = (i16)a->pcmSampleLatch;
            fifo->head               = next_head;
            if ((next_head + 1) % AUDIO_PCM_FIFO_SIZE == fifo->tail)
                a->globalStatus |= AUDIO_GLOB_PCM_FIFO_FULL;
        }

        a->pcmSampleLatch = 0;

        break;
    }
    default: break;
    }
};
