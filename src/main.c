/******************************************************************************
 *
 *   Taleä System emulator using raylib
 *
 ******************************************************************************/

#include "frontend/config.h"
#include "frontend/frontend.h"
#include "raylib.h"
#include "talea.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

extern void Serial_CloseSockets(TaleaMachine *m);
void        Machine_Poweroff(TaleaMachine *m)
{
    m->cpu.poweroff = true;
    Frontend_StopMusic();
    Frontend_PlayShutdown();
    Serial_CloseSockets(m);
}

void Machine_Init(TaleaMachine *m, TaleaConfig *conf)
{
    bool is_restart = Frontend_IsRestart();

    Frontend_StartMusic();
    Frontend_PlayStartup();

    Cpu_Reset(m, conf, is_restart);
    Terminal_Reset(m, conf, is_restart);
    Storage_Reset(m, conf, is_restart);
    Video_Reset(m, conf, is_restart);

    m->cpu.poweroff = false;

    if (is_restart) {
        OPLL_delete(m->synth);
    }

    m->synth = OPLL_new(SYNTH_MSX_CLK, 44100);

    m->sys = (DeviceSystem){
        .frequency     = 0,
        .unixtime_mode = false,
        .counter_mode  = false,
        .uptime        = 0,
    };

    // Register devices
    const int device_registry[] = {
        ID_TERMINAL, ID_VIDEO, ID_STORAGE, ID_AUDIO,
        ID_AUDIO,    ID_AUDIO, ID_AUDIO,   ID_MOUSE,
    };

    Bus_RegisterDevices(m, device_registry, 0, 7);

    Machine_LoadFirmware(m, conf->firmware_path);
}

void Machine_LoadFirmware(TaleaMachine *m, const char *path)
{
    int firmware_size;
    u8 *firmware;

    firmware_size = -1;
    firmware      = LoadFileData(path, &firmware_size);

    if (firmware == NULL || firmware_size < 1 || firmware_size > 8191) {
        TALEA_LOG_ERROR("Error, firmware file is too big or not valid");
        exit(1);
    }

    memcpy(&m->main_memory[0xFFE000], firmware, firmware_size);
    UnloadFileData(firmware);
}

void Machine_Deinit(TaleaMachine *m, TaleaConfig *config)
{
    OPLL_delete(m->synth);
    // Storage_UnloadResources(m);

    m = NULL; // Do we null the pointer or not
}

void Machine_RunFrame(TaleaMachine *m, TaleaConfig *config)
{
    m->cpu.frequency = (10 * HZ) * config->frequency;
    u32 cycle_quota  = m->cpu.frequency / FPS;

    if (config->dynamic_cycles) {
        float frameTime = GetFrameTime();
        cycle_quota += (frameTime <= FRAME_MS) ? 1000 : -1000;
    }

    if (!Frontend_GuiIsOpen() && !m->cpu.poweroff) {
        Cpu_RunCycles(m, cycle_quota);
    }
}

static TaleaMachine talea = { 0 };
Tps *TpsDrives = talea.storage.tps_drives; // To access in other threads
Hcs *HcsDrive  = &talea.storage.hcs;       // To access in other threads

int main(void)
{
    TaleaConfig config = Config_Load(CONFIG_FILE_PATH);

    // Initialization
    bool success = NetworkInit(); // TODO: check config for serial and modem
                                  // enable, and if it fails, log the error and
                                  // continue offline
    Frontend_InitWindow(&config);
    Machine_Init(&talea, &config);

    // Program loop
    while (!Frontend_ShouldClose()) {
        // 1. Process Input
        Frontend_SetupFrame(&talea, &config);

        // 2. Emulate (Calculate how many cycles to run based on DeltaTime)
        Machine_RunFrame(&talea, &config);

        if (!talea.cpu.poweroff) {
            Frontend_PollInput(&talea, &config);
            if (!Frontend_GuiIsOpen()) Video_Update(&talea);
            Serial_Update(&talea);
        }

        // 3. Render
        Frontend_RenderFrame(&talea, &config);
    }

    // De-Initialization
    Machine_Deinit(&talea, &config);
    Frontend_Deinit(&config);
    Storage_Deinit(&talea);
    NetworkDeinit();

    Config_Store(&config, CONFIG_FILE_PATH);
    return (0);
}