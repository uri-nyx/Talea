#include <stdio.h>

#include "config.h"
#include "raylib.h"
#include "talea.h"
#include "toml-c.h"

TaleaConfig Config_Load(const char *config_path)
{
    char          errbuf[200];
    char         *configFile;
    toml_table_t *tbl;

    configFile = LoadFileText(config_path);
    tbl        = toml_parse(configFile, errbuf, sizeof(errbuf));

    if (!tbl) {
        // TODO: If there is no config file, load sane defaults
        TALEA_LOG_ERROR("ERROR: %s\n", errbuf);
        return (TaleaConfig){ 0 };
    }

    toml_value_t dynamic_cycles = toml_table_bool(tbl, "Dynamic_Cycles");
    toml_value_t window_size    = toml_table_string(tbl, "Screen");
    toml_value_t crt_shader     = toml_table_bool(tbl, "CRT_shader");
    toml_value_t keep_TPS       = toml_table_bool(tbl, "Keep_TPS_loaded");
    toml_value_t TPS_A_path     = toml_table_string(tbl, "TPS_A");
    toml_value_t TPS_B_path     = toml_table_string(tbl, "TPS_B");
    toml_value_t sync_on_eject  = toml_table_bool(tbl, "Sync_TPS_when_ejected");
    toml_value_t hardware_font  = toml_table_string(tbl, "Hardware_font");
    toml_value_t frequency      = toml_table_double(tbl, "Speed");
    toml_value_t firmware_path  = toml_table_string(tbl, "Firmware");
    toml_value_t krepeat        = toml_table_bool(tbl, "Key_Repeat");
    toml_value_t serial_enable  = toml_table_bool(tbl, "Enable_Serial");
    toml_value_t serial_port    = toml_table_int(tbl, "Port");
    toml_value_t vsync          = toml_table_bool(tbl, "V-Sync");
    toml_value_t hidpi          = toml_table_bool(tbl, "HiDPI");

    if (!dynamic_cycles.ok) // Default values.
        dynamic_cycles.u.b = false;
    if (!window_size.ok) window_size.u.s = "windowed";
    if (!hardware_font.ok) hardware_font.u.s = "default.fnt";
    if (!firmware_path.ok) {
        firmware_path.u.s = "resources/emulated/firmware/bios.bin";
    }
    if (!crt_shader.ok) crt_shader.u.b = true;
    if (!sync_on_eject.ok) sync_on_eject.u.b = true;
    if (!keep_TPS.ok) keep_TPS.u.b = true;
    if (!krepeat.ok) krepeat.u.b = false;
    if (!serial_enable.ok) serial_enable.u.b = true;
    if (!vsync.ok) vsync.u.b = true;
    if (!hidpi.ok) hidpi.u.b = false;
    if (!TPS_A_path.ok) TPS_A_path.u.s = 0;
    if (!TPS_B_path.ok) TPS_B_path.u.s = 0;
    if (!frequency.ok) frequency.u.d = 0.2f;
    if (!serial_port.ok) serial_port.u.i = 1212;

    TaleaConfig config = { 0 };

    config.dynamic_cycles = dynamic_cycles.u.b;
    config.crt_shader     = crt_shader.u.b;
    config.sync_on_eject  = sync_on_eject.u.b;
    config.vsync          = vsync.u.b;
    config.hidpi          = hidpi.u.b;
    config.keep_TPS       = keep_TPS.u.b;
    config.TPS_A_path     = TPS_A_path.u.s;
    config.TPS_B_path     = TPS_B_path.u.s;
    config.hardware_font  = hardware_font.u.s;
    config.frequency      = frequency.u.d;
    config.firmware_path  = firmware_path.u.s;
    config.krepeat        = krepeat.u.b;
    config.serial_port    = serial_port.u.i;
    config.serial_enable  = serial_enable.u.b; // PUT IN INTERFACE

    if (TextIsEqual(window_size.u.s, "fullscreen"))
        config.window_size = FULLSCREEN;
    else if (TextIsEqual(window_size.u.s, "borderless"))
        config.window_size = BORDERLESS;
    else
        config.window_size = WINDOWED;

    toml_free(tbl);

    config.flags = config.vsync ? FLAG_VSYNC_HINT : 0;
    config.flags |= config.hidpi ? FLAG_WINDOW_HIGHDPI : 0;

    return config;
}

#define BOOL_STR(b) ((b) ? "true" : "false")
#define WSZ_STR(w)                        \
    ((w) == FULLSCREEN   ? "fullscreen" : \
     ((w) == BORDERLESS) ? "borderless" : \
                           "windowed")

bool Config_Store(const TaleaConfig *conf, const char *path)
{
    /* This function is dependent of stdio.h but can be rewritten with raylib
    string and file functions without problem */

    FILE *f      = fopen(path, "w");
    int   err    = 0;
    bool  retval = true;

    if (!f) {
        TALEA_LOG_ERROR("Could not open config file to save it: %s\n", path);
        return false;
    }

    err = fprintf(f, "# Configuration file for the Talea Computer System\n");
    if (err < 0) {
        retval = false;
    }

    err = fprintf(
        f,
        "Screen = \"%s\" # \"windowed\", \"fullscreen\", or \"borderless\"\n",
        WSZ_STR(conf->window_size));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "Dynamic_Cycles = %s\n", BOOL_STR(conf->dynamic_cycles));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "CRT_shader = %s\n", BOOL_STR(conf->crt_shader));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "Keep_TPS_loaded = %s\n", BOOL_STR(conf->keep_TPS));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "Sync_TPS_when_ejected = %s\n",
                  BOOL_STR(conf->sync_on_eject));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "Key_Repeat = %s\n", BOOL_STR(conf->krepeat));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "Enable_Serial = %s\n", BOOL_STR(conf->serial_enable));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "V-Sync = %s\n", BOOL_STR(conf->vsync));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "HiDPI = %s\n", BOOL_STR(conf->hidpi));
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "Port = %d\n", conf->serial_port);
    if (err < 0) {
        retval = false;
    }
    err = fprintf(f, "Speed = %f\n", conf->frequency);
    if (err < 0) {
        retval = false;
    }

    if (conf->firmware_path)
        err = fprintf(f, "Firmware = \"%s\"\n", conf->firmware_path);
    if (err < 0) {
        retval = false;
    }
    if (conf->TPS_A_path)
        err = fprintf(f, "TPS_A = \"%s\"\n", conf->TPS_A_path);
    if (err < 0) {
        retval = false;
    }

    if (conf->TPS_B_path)
        err = fprintf(f, "TPS_B = \"%s\"\n", conf->TPS_B_path);
    if (err < 0) {
        retval = false;
    }

    if (conf->hardware_font)
        err = fprintf(f, "Hardware_font = \"%s\"\n", conf->hardware_font);
    if (err < 0) {
        retval = false;
    }

    if (!retval) {
        TALEA_LOG_ERROR("Error writing config file\n");
    }

    fclose(f);

    return retval;
}