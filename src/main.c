/******************************************************************************
 *
 *   Taleä System emulator using raylib
 *
 ******************************************************************************/

#include "frontend/config.h"
#include "frontend/frontend.h"
#include "raylib.h"
#include "talea.h"
#include "core/bus.h" // Should move from here
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

static char *mapped_file_path = NULL; // A mapped file at 0x1000, passed in argv[0]

void Machine_Init(TaleaMachine *m, TaleaConfig *conf)
{
    bool is_restart = Frontend_IsRestart();

    Frontend_StartMusic();
    Frontend_PlayStartup();

    Bus_Reset(m);
    Cpu_Reset(m, conf, is_restart);
    Terminal_Reset(m, conf, is_restart);
    Storage_Reset(m, conf, is_restart);
    Video_Reset(m, conf, is_restart);
    Mouse_Reset(m, is_restart);

    m->cpu.poweroff = false;

    if (is_restart) {
        Frontend_StopSynth();
        OPLL_delete(m->synth.opll);
    }

    m->synth.opll = OPLL_new(SYNTH_MSX_CLK, 44100);
    Frontend_StartSynth();

    m->synth.master_volume = 100;

    m->sys = (DeviceSystem){
        .frequency     = 0,
        .unixtime_mode = false,
        .counter_mode  = false,
        .uptime        = 0,
    };

    // Register devices
    const int device_registry[5] = {
        ID_TERMINAL, ID_VIDEO, ID_STORAGE, ID_AUDIO, ID_MOUSE,
    };

    Bus_RegisterDevices(m, device_registry, 0, 4);

    if (mapped_file_path) {
        int sz   = 0;
        u8 *data = LoadFileData(mapped_file_path, &sz);
     
        if (data) {
            sz = MIN(sz, TALEA_MAIN_MEM_SZ - TALEA_MAPPED_FILE_ADDR);
            TaleaMemoryView view = Bus_GetView(m, TALEA_MAPPED_FILE_ADDR, sz, BUS_ACCESS_WRITE);
            size_t written = Bus_WriteBlock(m, data, &view, sz);
            if (written != sz) {
                TALEA_LOG_ERROR("Could not load mapped file at 0x%x\n", TALEA_MAPPED_FILE_ADDR);
            }
            UnloadFileData(data);
        }
    }

    Machine_LoadFirmware(m, conf->firmware_path);
}

void Machine_LoadFirmware(TaleaMachine *m, const char *path)
{
    int firmware_size;
    u8 *firmware;

    firmware_size = -1;
    firmware      = LoadFileData(path, &firmware_size);

    if (firmware == NULL || firmware_size < 1 || firmware_size > TALEA_MAX_FIRMWARE_SIZE) {
        TALEA_LOG_ERROR("Error, firmware file is too big or not valid (size: %d, max: %d)\n",
                        firmware_size, TALEA_MAX_FIRMWARE_SIZE);
        exit(1);
    }

    Bus_LoadFirmware(m, firmware, firmware_size);
    TALEA_LOG_TRACE("Loaded firmware at 0x%04x, size: %d bytes\n", TALEA_FIRMWARE_ADDRESS,
                    firmware_size);
    UnloadFileData(firmware);
}

void Machine_Deinit(TaleaMachine *m, TaleaConfig *config)
{
    Frontend_StopSynth();
    OPLL_delete(m->synth.opll);
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

static TaleaMachine talea     = { 0 };
Tps                *TpsDrives = talea.storage.tps_drives; // To access in other threads
Hcs                *HcsDrive  = &talea.storage.hcs;       // To access in other threads

int main(int argc, char **argv)
{
    if (argc > 2) {
        TALEA_LOG_ERROR("Usage: %s [optional file to map at 0x1000]\n", argv[0]);
    }

    SetTraceLogLevel(LOG_ALL);

    if (argc == 2) {
        mapped_file_path = argv[1];
    }

    TaleaConfig config = Config_Load(CONFIG_FILE_PATH);

    // Initialization
    bool success = NetworkInit(); // TODO: check config for serial and modem
                                  // enable, and if it fails, log the error and
                                  // continue offline
    Synth_Init(&talea);
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
            if (!Frontend_GuiIsOpen()) {
                Video_Update(&talea);
                Synth_Update(&talea);
            }
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