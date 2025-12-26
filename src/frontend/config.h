#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

enum WindowSize {
    WINDOWED,
    FULLSCREEN,
    BORDERLESS,
};

/* TOML fields:
    Screen: "windowed" | "fullscreen" | "borderless"
    Dynamic_Cycles: bool
    CRT_shader: bool
    V-Sync: bool
    HiDPI: bool
    Keep_TPS_loaded:bool
    Sync_TPS_when_ejected: bool
    Key_Repeat: bool
    Enable_Serial: bool
    Port: number
    Speed: float (slider)
    Firmware: string (path)
    TPS_A: string (path)
    TPS_B: string (path)
    Hardware_font: string (path)
*/

typedef struct TaleaConfig {
    bool dynamic_cycles, crt_shader, keep_TPS, sync_on_eject, krepeat,
        serial_enable, vsync, hidpi;
    enum WindowSize window_size;
    char           *firmware_path, *TPS_A_path, *TPS_B_path, *hardware_font;
    int             serial_port;
    float           frequency;
    int             flags;
} TaleaConfig;

TaleaConfig Config_Load(const char *config_path);

// Store configurations again to config file, so they persist between sessions.
// Returns true on success
bool Config_Store(const TaleaConfig *conf, const char *path);

#endif /* CONFIG_H */
