#include "audio.h"

static void WORD(char *buf, uint32_t data)
{
    buf[0] = data & 0xff;
    buf[1] = (data & 0xff00) >> 8;
}

void Synth_OPLL(void *buffer, unsigned int frames)
{
    char *wave = (char *)buffer;
    // printf("Generate %d frames", frames);

    for (size_t i = 0; i < frames; i++) {
        0; // WORD(wave + i * 2, OPLL_calc(opll));
    }
}

#if 0

u8
AudioHandlerReadU8(u16 addr) 
{
    u8 reg = addr - DEV_AUDIO_BASE;
	// DOES NOTHING
	return 0;
}

u16 AudioHandlerRead16 (u16 addr)
{
        u8 reg = addr - DEV_AUDIO_BASE;
        // DOES NOTHING
        return 0;
}

u32 AudioHandlerRead32(u16 addr)
{
        u8 reg = addr - DEV_AUDIO_BASE;
        // DOES NOTHING
        return 0;
}

void AudioHandlerWriteU8(u16 addr, u8 value) {
        u8 reg = addr - DEV_AUDIO_BASE;

        switch (reg) {
        case 0x39:
        OPLL_reset(opll);
        break;
        default:
        OPLL_writeReg(opll, reg, value);
        break;
        }
};

void AudioHandlerWrite16(u16 addr, u16 value)
{
        AudioHandlerWriteU8(addr, value >> 8);
        AudioHandlerWriteU8(addr + 1, value);
}
void AudioHandlerWrite32(u16 addr, u32 value)
{
        AudioHandlerWriteU8(addr, value >> 24);
        AudioHandlerWriteU8(addr + 1, value >> 16);
        AudioHandlerWriteU8(addr + 2, value >> 8);
        AudioHandlerWriteU8(addr + 3, value);
}
#endif