#ifndef FRONTEND_H
#define FRONTEND_H

#include "config.h"
#include "talea.h"

#include "gui_window_file_dialog.h"
#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
#define WINDOW_SCALE_DPI GetWindowScaleDPI()
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define WINDOW_SCALE_DPI (Vector2){ 1.0f, 1.0f }
#endif

struct FrontendWindowCtx {
    int             gameScreenWidth;
    int             gameScreenHeight;
    float           scale;
    RenderTexture2D screenTexture;
    Vector2         ScaleDPI;
};

struct FrontendUICtx {
    int                      activeFont;
    float                    frequencyMultiplier;
    float                    button_row;
    bool                     open_gui;
    bool                     hide_ui;
    bool                     dropdownFont;
    bool                     isTpsDialog, isFirmwareDialog;
    char                     firmwarePath[512];
    GuiWindowFileDialogState fileDialogState;
    Texture                  dialogTexture;
};

// SOUND EFFECTS
#define SFX_PATH \
    "resources" PATH_SEPARATOR "audio" PATH_SEPARATOR "sfx" PATH_SEPARATOR

#define SFX_STARTUP    SFX_PATH "startup.ogg"
#define SFX_SHUTDOWN   SFX_PATH "shutdown.ogg"
#define SFX_INSERT_TPS SFX_PATH "floppy_insert.ogg"
#define SFX_EJECT_TPS  SFX_PATH "floppy_eject.ogg"
#define SFX_ACCESS_TPS SFX_PATH "access" PATH_SEPARATOR
#define SFX_IDLE       SFX_PATH "idle.wav"

#define MAX_ACCESS_SFX 6

struct FrontendSFXCtx {
    int   nextStorageAccessSfx;
    bool  playAccessSfx;
    Sound tpsInsertSfx, tpsEjectSfx;
    Sound storageAcessSfx[MAX_ACCESS_SFX];
    Sound startupSfx;
    Sound shutdownSfx;
    Music loopSound;
};

struct FrontendCRTShaderCtx {
    int             texture_loc, time_loc;
    float           u_time;
    Shader          shader;
    RenderTexture2D viewport;
};

struct FrontendSynthCtx {
    AudioStream PCMstream;
};

struct FrontendState {
    struct FrontendWindowCtx    window;
    struct FrontendUICtx        ui;
    struct FrontendSFXCtx       sfx;
    struct FrontendCRTShaderCtx crt;
    struct FrontendSynthCtx     synth;
    bool                        is_restart;
};

void Frontend_InitWindow(TaleaConfig *config);
void Frontend_PollInput(TaleaMachine *m, TaleaConfig *config);
void Frontend_SetupFrame(TaleaMachine *m, TaleaConfig *config);
void Frontend_RenderFrame(TaleaMachine *m, TaleaConfig *config);
void Frontend_Deinit(TaleaConfig *config);

void Frontend_SetActiveFont(u8 font);

void Frontend_StartMusic(void);
void Frontend_PlayStartup(void);

void Frontend_StopMusic(void);
void Frontend_PlayShutdown(void);

struct FrontendState *Frontend_GetState(void);

bool Frontend_ShouldClose(void);
bool Frontend_GuiIsOpen(void);
bool Frontend_IsRestart(void);

#endif /* FRONTEND_H */
