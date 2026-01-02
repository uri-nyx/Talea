#include "frontend.h"
#include "config.h"
#include "devices/audio.h"
#include "talea.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "rlgl.h"
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct FrontendState state = { 0 };

static inline Rectangle GetScaledViewport(void)
{
    return ((Rectangle){
        (GetRenderWidth() - ((float)state.window.gameScreenWidth * state.window.scale)) *
            (0.5f / state.window.ScaleDPI.x),
        (GetRenderHeight() - (((float)state.window.gameScreenHeight - 5) * state.window.scale)) *
            (0.5f / state.window.ScaleDPI.y),
        (float)(state.window.gameScreenWidth) * (state.window.scale / state.window.ScaleDPI.x),
        (float)(state.window.gameScreenHeight - 35) *
            (state.window.scale / state.window.ScaleDPI.y) });
}

static inline void PlayNextStorageAccessSfx(void)
{
    PlaySound(state.sfx.storageAcessSfx[state.sfx.nextStorageAccessSfx]);
    if (++state.sfx.nextStorageAccessSfx >= MAX_ACCESS_SFX) state.sfx.nextStorageAccessSfx = 0;
}

void Frontend_InitWindow(TaleaConfig *config)
{
    int Ww = TALEA_WINDOW_WIDTH;
    int Wh = TALEA_WINDOW_HEIGHT;

#if defined(PLATFORM_DESKTOP) && defined(HACK_HIDPI)
    if (config->hidpi) {
        Ww *= HACK_HIDPI;
        Wh *= HACK_HIDPI;
    } 
#endif

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | config->flags); // TODO: Put in the
    // TODO: Put in the config file AND in the  UI
    InitWindow(Ww, Wh, TALEA_WINDOW_TITLE);

#if defined(PLATFORM_DESKTOP)
    SetWindowPosition((GetMonitorWidth(GetCurrentMonitor()) - GetRenderWidth()) / 2,
                      (GetMonitorHeight(GetCurrentMonitor()) - GetRenderHeight()) / 2);
    SetWindowMinSize(320, 240);
#endif

    InitAudioDevice();
    SetMasterVolume(1.0f); // TODO: Put in the config file AND in the UI
    SetAudioStreamBufferSizeDefault(4096);

    state = (struct FrontendState){
        .window =
            (struct FrontendWindowCtx){
                .gameScreenWidth  = TALEA_SCREEN_WIDTH,
                .gameScreenHeight = TALEA_SCREEN_HEIGHT,
                .scale            = 0,
                .ScaleDPI         = WINDOW_SCALE_DPI,
                .screenTexture    = LoadRenderTexture(TALEA_SCREEN_WIDTH, TALEA_SCREEN_HEIGHT),
            },
        .ui =
            (struct FrontendUICtx){
                .button_row          = 0,
                .activeFont          = 0,
                .frequencyMultiplier = config->frequency,
                .dropdownFont        = false,
                .open_gui            = false,
                .hide_ui             = false,
                .isFirmwareDialog    = false,
                .isTpsDialog         = false,
                .dialogTexture       = { 0 },
                .firmwarePath        = { 0 },
                .fileDialogState     = InitGuiWindowFileDialog(
                    TextFormat("%s" TPS_IMAGES_DEFAULT_DIR, GetWorkingDirectory())),
            },
        .sfx =
            (struct FrontendSFXCtx){
                .startupSfx           = LoadSound(SFX_STARTUP),
                .shutdownSfx          = LoadSound(SFX_SHUTDOWN),
                .tpsInsertSfx         = LoadSound(SFX_INSERT_TPS),
                .tpsEjectSfx          = LoadSound(SFX_EJECT_TPS),
                .loopSound            = LoadMusicStream(SFX_IDLE),
                .nextStorageAccessSfx = 0,
                .storageAcessSfx      = { 0 },
            },
        .crt =
            (struct FrontendCRTShaderCtx){
                .viewport    = LoadRenderTexture(TALEA_SCREEN_WIDTH * 2, TALEA_SCREEN_HEIGHT * 2),
                .shader      = LoadShader(VERTEX_SHADER, SHADERS_PATH("crt.fs")),
                .texture_loc = 0,
                .time_loc    = 0,
            },
        .synth =
            (struct FrontendSynthCtx){
                .PCMstream = LoadAudioStream(44100, 16, 1),
            },
        .is_restart = false,
    };

    state.window.scale  = MIN((float)GetRenderWidth() / (state.window.gameScreenWidth),
                              ((float)GetRenderHeight() / (state.window.gameScreenHeight)));
    state.ui.button_row = ((float)GetRenderHeight() / state.window.scale) *
                          (state.window.scale / state.window.ScaleDPI.y);
    GuiLoadStyle("resources/terminal.rgs"); // TODO: put in config file
    strncpy(state.ui.firmwarePath, config->firmware_path, 512);

    for (size_t i = 0; i < MAX_ACCESS_SFX; i++) {
        state.sfx.storageAcessSfx[i] =
            LoadSound(TextFormat("%sfloppy_access%d.ogg", SFX_ACCESS_TPS, i + 1));
        SetSoundVolume(state.sfx.storageAcessSfx[i], 0.07f);
    }

    SetSoundVolume(state.sfx.tpsInsertSfx, 0.2f);
    SetSoundVolume(state.sfx.tpsEjectSfx, 0.2f);
    SetMusicVolume(state.sfx.loopSound, 0.5f);

    SetAudioStreamCallback(state.synth.PCMstream, Synth_OPLL);
    PlayAudioStream(state.synth.PCMstream);

    SetTextureFilter(state.window.screenTexture.texture, TEXTURE_FILTER_POINT);

    state.crt.texture_loc = GetShaderLocation(state.crt.shader, "texture1");
    state.crt.time_loc    = GetShaderLocation(state.crt.shader, "iTime");

    SetTargetFPS(FPS); // TODO: put in condig file
    SetExitKey(KEY_NULL);
}

static u16 GetPressedModifiers()
{
    u16 mod = 0;

    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        mod |= SHIFT;
    } else {
        mod &= ~SHIFT;
    }

    if (IsKeyDown(KEY_RIGHT_ALT)) {
        mod |= RALT;
    } else {
        mod &= ~RALT;
    }

    if (IsKeyDown(KEY_LEFT_ALT)) {
        mod |= LALT;
    } else {
        mod &= ~LALT;
    }

    if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) {
        mod |= RGUI;
    } else {
        mod &= ~RGUI;
    }

    if (IsKeyDown(KEY_CAPS_LOCK)) {
        mod |= CAPSLOCK;
    } else {
        mod &= ~CAPSLOCK;
    }

    if (IsKeyDown(KEY_SCROLL_LOCK)) {
        mod |= SCROLLLOCK;
    } else {
        mod &= ~SCROLLLOCK;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        mod |= CONTROL;
    } else {
        mod &= ~CONTROL;
    }

    return mod;
}

static void Frontend_PollKeyboard(TaleaMachine *m, TaleaConfig *config)
{
    static int prev_key = 0;
    static u8  prev_chr = 0;
    static u16 prev_mod = 0;
    int        key;
    u8         chr;
    bool       is_down;

    u16 mod = GetPressedModifiers();

    if (config->krepeat && IsKeyPressedRepeat(prev_key)) {
        key     = prev_key;
        chr     = prev_chr;
        is_down = IsKeyDown(key);
    } else {
        key     = GetKeyPressed();
        chr     = GetCharPressed();
        is_down = IsKeyDown(key);
    }

    bool emulator_control = false;

    if (key) {
        // Maybe use a table
        switch (key) {
        case KEY_ENTER: chr = '\r'; break;
        case KEY_TAB: chr = '\t'; break;
        case KEY_BACKSPACE: chr = '\b'; break;
        case KEY_ESCAPE: chr = 27; break;
        case KEY_UP: chr = 28; break;
        case KEY_DOWN: chr = 29; break;
        case KEY_LEFT: chr = 30; break;
        case KEY_RIGHT: chr = 31; break;
        case KEY_DELETE: chr = 127; break;
        /* EMULATOR GUI CONTROLS */
        case KEY_F9:
            emulator_control = true;
            state.ui.hide_ui = !state.ui.hide_ui;
            break;
        case KEY_F10:
            emulator_control  = true;
            state.ui.open_gui = !state.ui.open_gui;
            break;
        default: break;
        }
        prev_key = key;

        if (mod & CONTROL && key >= KEY_A && key <= KEY_RIGHT_BRACKET) {
            chr = key & 0x1f;
        }

        prev_chr = chr;
    }

    if (!state.ui.open_gui && !emulator_control) {
        if (IsKeyReleased(prev_key))
            Keyboard_ProcessKeypress(m, false, prev_key, prev_chr, prev_mod);
        if (IsKeyPressed(key) || (config->krepeat && IsKeyPressedRepeat(key)))
            Keyboard_ProcessKeypress(m, true, key, chr, mod);
    }

    prev_mod = mod;
}

static void Frontend_PollMouse(TaleaMachine *m, TaleaConfig *config)
{
    Vector2   positionMouse = GetMousePosition();
    Rectangle viewport      = GetScaledViewport();

    int scaled_x =
        (int)(positionMouse.x - (GetRenderWidth() - viewport.width) / 2) * state.window.scale;
    int scaled_y =
        (int)(positionMouse.y - (GetRenderWidth() - viewport.height) / 2) * state.window.scale;

    u8 current_state = 0;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) &&
        CheckCollisionPointRec(positionMouse, GetScaledViewport())) {
        current_state |= MOUSE_BUTT_RIGHT; // TODO: use enum for this
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(positionMouse, GetScaledViewport())) {
        current_state |= MOUSE_BUTT_LEFT;
    }

    static uint8_t last_state = 0;
    if (current_state != last_state) {
        Mouse_ProcessButtonPress(m, current_state, scaled_x, scaled_y);
        last_state = current_state;
    }

    Mouse_UpdateCoordinates(m, current_state, scaled_x, scaled_y);
}

static void Frontend_PollSerial(TaleaMachine *m, TaleaConfig *config)
{
    if (config->serial_enable) {
        TALEA_LOG_ERROR("Netwok not implemented!\n");
    }
}

void Frontend_PollInput(TaleaMachine *m, TaleaConfig *config)
{
    Frontend_PollKeyboard(m, config);

    if (state.ui.open_gui) return;
    Frontend_PollMouse(m, config);
    Frontend_PollSerial(m, config);
}

float u_time = 0;
void  Frontend_SetupFrame(TaleaMachine *m, TaleaConfig *config)
{
    config->frequency = state.ui.frequencyMultiplier;
    u_time            = (float)GetTime();
    SetShaderValue(m->video.renderer.shader, m->video.renderer.time_loc, &u_time,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(state.crt.shader, state.crt.time_loc, &u_time, SHADER_UNIFORM_FLOAT);

    if (!IsShaderValid(m->video.renderer.shader)) TALEA_LOG_ERROR("Renderer Shader is not valid");
    if (!IsShaderValid(state.crt.shader)) TALEA_LOG_ERROR("CRT Shader is not valid");

    state.window.scale  = MIN((float)GetRenderWidth() / (state.window.gameScreenWidth),
                              ((float)GetRenderHeight() / (state.window.gameScreenHeight)));
    state.ui.button_row = ((float)GetRenderHeight() / state.window.scale) *
                          (state.window.scale / state.window.ScaleDPI.y);
}

static void RenderGUI(TaleaMachine *m, TaleaConfig *config)
{
    static enum TpsId tpsToLoad;

    BeginDrawing();
    if (state.ui.fileDialogState.SelectFilePressed) {
        if (state.ui.isTpsDialog) {
            bool success = Storage_InsertTps(tpsToLoad, state.ui.fileDialogState.fileNameText);
            if (!success)
                TALEA_LOG_ERROR("Error Loading TPS\n");
            else
                PlaySound(state.sfx.tpsInsertSfx);
        } else if (state.ui.isFirmwareDialog) {
            bool writeProtected = false;
            // Load firmware file (if supported extension)
            // Check this for the restarts
            if (IsFileExtension(state.ui.fileDialogState.fileNameText, ".bin")) {
                strcpy(state.ui.firmwarePath,
                       TextFormat("%s" PATH_SEPERATOR "%s", state.ui.fileDialogState.dirPathText,
                                  state.ui.fileDialogState.fileNameText));

                strcpy(config->firmware_path, state.ui.firmwarePath);
                state.is_restart = true;
                Machine_Init(m, config);
            } else {
                TALEA_LOG_WARNING("Can only load absolute binaries as firmware");
            }
        }

        state.ui.isFirmwareDialog                  = false;
        state.ui.isTpsDialog                       = false;
        state.ui.fileDialogState.SelectFilePressed = false;
    }

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawTexture(state.ui.dialogTexture, GetScreenWidth() / 2 - state.ui.dialogTexture.width / 2,
                GetScreenHeight() / 2 - state.ui.dialogTexture.height / 2 - 5, WHITE);
    DrawRectangleLines(GetScreenWidth() / 2 - state.ui.dialogTexture.width / 2,
                       GetScreenHeight() / 2 - state.ui.dialogTexture.height / 2 - 5,
                       state.ui.dialogTexture.width, state.ui.dialogTexture.height, BLACK);

    // raygui: controls drawing
    //----------------------------------------------------------------------------------
    if (state.ui.fileDialogState.windowActive) {
        GuiLock();
    }

    GuiToggle((Rectangle){ 320, 20, 160, 30 }, "Sync TPS on eject", &config->sync_on_eject);
    GuiToggle((Rectangle){ 320, 60, 160, 30 }, "Dynamic cycle quota", &config->dynamic_cycles);
    GuiToggle((Rectangle){ 320, 100, 160, 30 }, "CRT simulation", &config->crt_shader);

    if (GuiButton((Rectangle){ 320, 140, 160, 30 }, "Toggle full screen")) {
        ToggleFullscreen();
    }

    if (GuiButton((Rectangle){ 320, 180, 160, 30 }, "Toggle borderless")) {
        ToggleBorderlessWindowed();
    }

    GuiSlider((Rectangle){ 320, 220, 160, 30 }, "10 Mhz", "100Mhz", &state.ui.frequencyMultiplier,
              0.1f, 1.0f);

    if (GuiButton((Rectangle){ 20, 20, 140, 30 },
                  GuiIconText(ICON_FILE_SAVE_CLASSIC, "Load TPS A"))) {
        state.ui.fileDialogState.windowActive = true;
        state.ui.isTpsDialog                  = true;
        tpsToLoad                             = TPS_ID_A; // <-
    }

    if (GuiButton((Rectangle){ 170, 20, 140, 30 },
                  GuiIconText(ICON_FILE_SAVE_CLASSIC, "Load TPS B"))) {
        state.ui.fileDialogState.windowActive = true;
        state.ui.isTpsDialog                  = true;
        tpsToLoad                             = TPS_ID_B;
    }

    if (GuiButton((Rectangle){ 20, 60, 140, 30 }, GuiIconText(ICON_CROSS, "Eject TPS A"))) {
        Storage_EjectTps(TPS_ID_A);
        PlaySound(state.sfx.tpsEjectSfx);
    }

    if (GuiButton((Rectangle){ 170, 60, 140, 30 }, GuiIconText(ICON_CROSS, "Eject TPS B"))) {
        Storage_EjectTps(TPS_ID_B);
        PlaySound(state.sfx.tpsEjectSfx);
    }

    if (GuiDropdownBox((Rectangle){ 20, 100, 140, 30 },
                       "Font 0;"
                       "Font 1;"
                       "Font 2;"
                       "Font 3;"
                       "Font 4;"
                       "Font 5;",
                       &state.ui.activeFont, state.ui.dropdownFont)) {
        // TODO: do this with a list from the config file
        state.ui.dropdownFont = !state.ui.dropdownFont;
        if (!state.ui.dropdownFont)
            TALEA_LOG_WARNING("Should change font\n"); // VideoChangeFont(state.ui.activeFont);
    }

    if (GuiButton((Rectangle){ 170, 100, 140, 30 }, "Load Firmware")) {
        state.ui.fileDialogState.windowActive = true;
        state.ui.isFirmwareDialog             = true;
        if (!state.ui.dropdownFont) TALEA_LOG_WARNING("Should change font\n");
        ; // VideoChangeFont(state.ui.activeFont);
    }

    if (GuiButton((Rectangle){ 10, (state.ui.button_row) - 30, 140, 30 },
                  GuiIconText(ICON_PLAYER_PLAY, "Resume [F10]"))) {
        state.ui.open_gui = false;
    }

    GuiUnlock();
    GuiWindowFileDialog(&state.ui.fileDialogState);

    // TODO: Put FPS option in config file
    DrawFPS(((float)GetRenderWidth() / state.window.scale) *
                    (state.window.scale / WINDOW_SCALE_DPI.x) -
                80,
            (int)state.ui.button_row - 25);
    // DrawTexture(overlay, 0, 0, WHITE);
    EndDrawing();
}

static void RenderScreen(TaleaMachine *m, TaleaConfig *config)
{
    BeginDrawing();
    ClearBackground(BLACK); // Clear screen background

    if (m->cpu.poweroff) {
        BeginTextureMode(state.window.screenTexture);
        ClearBackground(DARKGRAY);
        EndTextureMode();
    }

    if (config->crt_shader) {
        BeginShaderMode(state.crt.shader);

        SetShaderValueTexture(state.crt.shader, state.crt.texture_loc,
                              state.window.screenTexture.texture);

        DrawTexturePro(state.crt.viewport.texture,
                       (Rectangle){ 0.0f, 0.0f, (float)state.crt.viewport.texture.width,
                                    (float)-state.crt.viewport.texture.height },
                       GetScaledViewport(), (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndShaderMode();
    } else {
        DrawTexturePro(state.window.screenTexture.texture,
                       (Rectangle){ 0.0, 0.0f, (float)state.window.screenTexture.texture.width,
                                    (float)-state.window.screenTexture.texture.height },
                       GetScaledViewport(), (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    if (state.ui.hide_ui) goto end;

    if (GuiButton((Rectangle){ 610, (state.ui.button_row) - 30, 140, 30 },
                  (m->cpu.poweroff) ? GuiIconText(ICON_PLAYER_PLAY, "Start") :
                                      GuiIconText(ICON_PLAYER_STOP, "Shutdown"))) {
        if (m->cpu.poweroff) {
            state.is_restart = true;
            Machine_Init(m, config);
        } else {
            Machine_Poweroff(m);
        }
    }
    if (GuiButton((Rectangle){ 10, (state.ui.button_row) - 30, 140, 30 },
                  GuiIconText(ICON_PLAYER_PAUSE, "Pause [F10]")))
        state.ui.open_gui = true;
    if (GuiButton((Rectangle){ 460, (state.ui.button_row) - 30, 140, 30 }, "Hide UI [F9]"))
        state.ui.hide_ui = true;
end:
    DrawFPS(((float)GetRenderWidth() / state.window.scale) *
                    (state.window.scale / WINDOW_SCALE_DPI.x) -
                80,
            (int)state.ui.button_row - 25);

    EndDrawing();
}

void Frontend_RenderFrame(TaleaMachine *m, TaleaConfig *config)
{
    if (state.ui.open_gui) {
        RenderGUI(m, config);
    } else {
        RenderScreen(m, config);
    }
}

bool Frontend_ShouldClose()
{
    return WindowShouldClose();
}

bool Frontend_GuiIsOpen()
{
    return state.ui.open_gui;
}

bool Frontend_IsRestart()
{
    return state.is_restart;
}

struct FrontendState *Frontend_GetState(void)
{
    return &state;
}

void Frontend_SetActiveFont(u8 font)
{
    state.ui.activeFont = font;
}

void Frontend_Deinit(TaleaConfig *config)
{
    UnloadTexture(state.ui.dialogTexture);
    UnloadRenderTexture(state.window.screenTexture);
    UnloadSound(state.sfx.startupSfx);
    UnloadSound(state.sfx.tpsEjectSfx);
    UnloadSound(state.sfx.tpsInsertSfx);
    UnloadMusicStream(state.sfx.loopSound);
    UnloadAudioStream(state.synth.PCMstream);
    for (size_t i = 0; i < MAX_ACCESS_SFX; i++) {
        UnloadSound(state.sfx.storageAcessSfx[i]);
    }
    CloseAudioDevice();
    CloseWindow();
}

void Frontend_StopMusic(void)
{
    StopMusicStream(state.sfx.loopSound);
};

void Frontend_PlayShutdown(void)
{
    PlaySound(state.sfx.shutdownSfx);
}

void Frontend_StartMusic(void)
{
    PlayMusicStream(state.sfx.loopSound);
}

void Frontend_PlayStartup(void)
{
    PlaySound(state.sfx.startupSfx);
}