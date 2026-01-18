#include "audio.h"
#include "core/bus.h"

#define P_AUDIO_ADDR           0x0
#define P_AUDIO_DATA           0x1
#define P_AUDIO_CSR            0x2
#define P_AUDIO_FNUMH          0x3
#define P_AUDIO_FNUML          0x4
#define P_AUDIO_DURH           0x5
#define P_AUDIO_DURL           0x6
#define P_AUDIO_CHANNEL_SELECT 0x7
#define P_AUDIO_GLOBAL_STATUS0 0x8
#define P_AUDIO_GLOBAL_STATUS1 0x9
#define P_AUDIO_GLOBAL_STATUS2 0xa
#define P_AUDIO_GLOBAL_STATUS3 0xb
#define P_AUDIO_MASTER_VOL     0xc
#define P_AUDIO_PCM_FIFOH      0xd
#define P_AUDIO_PCM_FIFOL      0xe

// ugly hack to be able to pass the synth to the callback
static DeviceSynth *synth = NULL;

void Synth_Init(TaleaMachine *m)
{
    // maybe this is all over the place, but the frontend takes care of the audio stream
    // initialization here se simply assign the static pointer to be a reference to the machine's
    // synth.

    synth = &m->synth;
}

static inline void clearBusy(DeviceSynth *synth, u8 ch)
{
    synth->channels[ch].csr &= ~AUDIO_CSR_BUSY;
    synth->global_status &= ~(1 << (ch + SYNTH_NUM_CHANNELS));
}

static inline void setBusy(DeviceSynth *synth, u8 ch)
{
    synth->channels[ch].csr |= AUDIO_CSR_BUSY;
    synth->global_status |= (1 << (ch + SYNTH_NUM_CHANNELS));
}

static inline void clearNoteEnded(DeviceSynth *synth, u8 ch)
{
    synth->channels[ch].csr &= ~AUDIO_CSR_NOTE_ENDED;
    synth->global_status &= ~(1 << ch);
}

static inline void setNoteEnded(DeviceSynth *synth, u8 ch)
{
    synth->channels[ch].csr |= AUDIO_CSR_NOTE_ENDED;
    synth->global_status |= (1 << ch);
}

void Synth_PCM(void *buffer, unsigned int frames)
{
    if (!synth) return;

    i16 *wave         = (i16 *)buffer;
    u8   volume_lut[] = { 0, 2, 4, 8, 16, 24, 32, 48, 64, 80, 96, 128, 160, 192, 224, 255 };
    i32  mult         = volume_lut[synth->master_volume & 0x0F];
    u16  pcm_count;

    if (synth->pcm_fifo.head >= synth->pcm_fifo.tail) {
        pcm_count = synth->pcm_fifo.head - synth->pcm_fifo.tail;
    } else {
        pcm_count = (PCM_FIFO_SIZE - synth->pcm_fifo.tail) + synth->pcm_fifo.head;
    }

    for (size_t i = 0; i < frames; i++) {
        if (synth->pcm_fifo.head != synth->pcm_fifo.tail) {
            i16 sample           = synth->pcm_fifo.buffer[synth->pcm_fifo.tail];
            i32 mixed_sample     = ((i32)sample * mult) >> 8;
            mixed_sample         = mixed_sample > INT16_MAX ? INT16_MAX :
                                   mixed_sample < INT16_MIN ? INT16_MIN :
                                                              mixed_sample;
            wave[i]              = (i16)mixed_sample;
            synth->pcm_fifo.tail = (synth->pcm_fifo.tail + 1) % PCM_FIFO_SIZE;
            synth->global_status &= ~AUDIO_GLOB_PCM_FIFO_FULL;
           
            pcm_count--;

            if (pcm_count < (PCM_FIFO_SIZE / 2))
                synth->global_status |= AUDIO_GLOB_PCM_LOW_WATERMARK;
        } else {
            wave[i] = 0; // Silence if FIFO is empty
        }
    }
}

void Synth_OPLL(void *buffer, unsigned int frames)
{
    if (!synth || !synth->opll) return;

    i16 *wave = (i16 *)buffer;

    for (size_t i = 0; i < SYNTH_NUM_CHANNELS; i++) {
        SynthChannel *chan = &synth->channels[i];

        if (chan->sequencer_active) {
            chan->samples_left -= frames;

            if (chan->samples_left <= 0) {
                OPLL_writeReg(synth->opll, 0x20 + i, chan->last_freq_high & 0xEF);

                if (chan->csr & AUDIO_CSR_LOOP) {
                    chan->samples_left = chan->total_samples;
                    OPLL_writeReg(synth->opll, 0x20 + i, 0x10 | chan->last_freq_high);

                    if (chan->csr & AUDIO_CSR_GATE) {
                        chan->samples_left = 0;
                        OPLL_writeReg(synth->opll, 0x20 + i, chan->last_freq_high & 0xEF);
                    }
                } else {
                    chan->sequencer_active = false;
                    clearBusy(synth, i);
                    setNoteEnded(synth, i);

                    if (chan->csr & AUDIO_CSR_IE) {
                        synth->fire_interrupt = true;
                    }
                }
            }
        }
    }

    u8  volume_lut[] = { 0, 2, 4, 8, 16, 24, 32, 48, 64, 80, 96, 128, 160, 192, 224, 255 };
    i32 mult         = volume_lut[synth->master_volume & 0x0F];
    for (size_t i = 0; i < frames; i++) {
        i16 sample       = OPLL_calc(synth->opll);
        i32 mixed_sample = ((i32)sample * mult) >> 8;
        mixed_sample     = mixed_sample > INT16_MAX ? INT16_MAX :
                           mixed_sample < INT16_MIN ? INT16_MIN :
                                                      mixed_sample;
        wave[i]          = (i16)mixed_sample;
    }
}

void Synth_Update(TaleaMachine *m)
{
    if (m->synth.fire_interrupt) {
        m->synth.fire_interrupt = false;
        Machine_RaiseInterrupt(m, INT_AUDIO_NOTE_END, PRIORITY_AUDIO_INTERRUPT);
    }
}

u8 Synth_ReadHandler(TaleaMachine *m, u16 addr)
{
    u8 reg = addr - DEV_AUDIO_BASE;
    u8 ch  = m->synth.selected_channel % SYNTH_NUM_CHANNELS;

    u16 pcm_count;
    if (synth->pcm_fifo.head >= synth->pcm_fifo.tail) {
        pcm_count = synth->pcm_fifo.head - synth->pcm_fifo.tail;
    } else {
        pcm_count = (PCM_FIFO_SIZE - synth->pcm_fifo.tail) + synth->pcm_fifo.head;
    }

    switch (reg) {
    case P_AUDIO_ADDR: return 0;
    case P_AUDIO_DATA: return 0;
    case P_AUDIO_CSR: return m->synth.channels[ch].csr;
    case P_AUDIO_FNUMH: return m->synth.channels[ch].fnum >> 8;
    case P_AUDIO_FNUML: return m->synth.channels[ch].fnum;
    case P_AUDIO_DURH: return m->synth.channels[ch].dur >> 8;
    case P_AUDIO_DURL: return m->synth.channels[ch].dur;
    case P_AUDIO_CHANNEL_SELECT: return ch;
    case P_AUDIO_GLOBAL_STATUS0: return 0; break; // unused
    case P_AUDIO_GLOBAL_STATUS1: return m->synth.global_status >> 16; break;
    case P_AUDIO_GLOBAL_STATUS2: return m->synth.global_status >> 8; break;
    case P_AUDIO_GLOBAL_STATUS3: return m->synth.global_status; break;
    case P_AUDIO_MASTER_VOL: return m->synth.master_volume; break;
    case P_AUDIO_PCM_FIFOH: return pcm_count >> 8; break;
    case P_AUDIO_PCM_FIFOL: return pcm_count; break;
    default: return 0;
    }
}

void Synth_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    DeviceSynth  *synth = &m->synth;
    SynthChannel *chan  = &synth->channels[m->synth.selected_channel % SYNTH_NUM_CHANNELS];
    u8            ch_id = m->synth.selected_channel % SYNTH_NUM_CHANNELS;
    u8            reg   = addr - DEV_AUDIO_BASE;

    switch (reg) {
    case P_AUDIO_ADDR:
    case P_AUDIO_DATA: OPLL_writeIO(synth->opll, reg, value); break;
    case P_AUDIO_CSR:
        if (value & AUDIO_CSR_TRIGGER && !(chan->csr & AUDIO_CSR_BUSY)) {
            value &= ~AUDIO_CSR_TRIGGER;
            value &= ~AUDIO_CSR_NOTE_ENDED;
            clearNoteEnded(synth, ch_id);
            value |= AUDIO_CSR_BUSY;
            setBusy(synth, ch_id);

            chan->sequencer_active = true;

            chan->total_samples = (chan->dur * SYNTH_SAMPLERATE) / 1000;
            chan->samples_left  = chan->total_samples;

            chan->last_freq_low  = chan->fnum & 0xFF;
            chan->last_freq_high = chan->fnum >> 8;

            OPLL_writeReg(synth->opll, 0x10 + ch_id, chan->last_freq_low);
            OPLL_writeReg(synth->opll, 0x20 + ch_id, 0x10 | chan->last_freq_high);
            if (value & AUDIO_CSR_GATE) {
                chan->samples_left = 0;
                OPLL_writeReg(synth->opll, 0x10 + ch_id, chan->last_freq_low);
                OPLL_writeReg(synth->opll, 0x20 + ch_id, 0xef & chan->last_freq_high);
            }
        }

        if (value & AUDIO_CSR_STOP) {
            clearBusy(synth, ch_id);
            value &= ~AUDIO_CSR_BUSY;
            value &= ~AUDIO_CSR_STOP;
            chan->sequencer_active = false;
            OPLL_writeReg(synth->opll, 0x20 + ch_id, chan->last_freq_high & 0xEF);
        }

        chan->csr = value;

        break;
    case P_AUDIO_FNUMH: chan->fnum = (chan->fnum & 0x00FF) | (u16)value << 8; break;
    case P_AUDIO_FNUML: chan->fnum = (chan->fnum & 0xFF00) | value; break;
    case P_AUDIO_DURH: chan->dur = (chan->dur & 0x00FF) | (u16)value << 8; break;
    case P_AUDIO_DURL: chan->dur = (chan->dur & 0xFF00) | value; break;
    case P_AUDIO_CHANNEL_SELECT: synth->selected_channel = value % SYNTH_NUM_CHANNELS; break;
    case P_AUDIO_GLOBAL_STATUS0: break;
    case P_AUDIO_GLOBAL_STATUS1:
        synth->global_status = (synth->global_status & 0x00ffff) | (u32)value << 16;
        break;
    case P_AUDIO_GLOBAL_STATUS2:
        synth->global_status = (synth->global_status & 0xff00ff) | (u32)value << 8;
        break;
    case P_AUDIO_GLOBAL_STATUS3:
        synth->global_status = (synth->global_status & 0xffff00) | (u32)value;
        break;
    case P_AUDIO_MASTER_VOL: synth->master_volume = value; break;
    case P_AUDIO_PCM_FIFOH: synth->pcm_sample_latch = (u16)value << 8; break;
    case P_AUDIO_PCM_FIFOL: {
        // Only when we write to here, we push
        PCMFifo *fifo = &synth->pcm_fifo;

        synth->pcm_sample_latch |= (value & 0xff);

        size_t next_head = (fifo->head + 1) % PCM_FIFO_SIZE;
        if (next_head != fifo->tail) {
            fifo->buffer[fifo->head] = (i16)synth->pcm_sample_latch;
            fifo->head               = next_head;
            if ((next_head + 1) % PCM_FIFO_SIZE == fifo->tail)
                synth->global_status |= AUDIO_GLOB_PCM_FIFO_FULL;
        }

        synth->pcm_sample_latch = 0;

        break;
    }
    default: break;
    }
};
