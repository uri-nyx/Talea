#include <string.h>

#include "core/bus.h"
#include "frontend/frontend.h"
#include "palettes.h"
#include "talea.h"

// PORTS

#define P_VIDEO_COMMAND (DEV_VIDEO_BASE + 0)
#define P_VIDEO_GPU0    (DEV_VIDEO_BASE + 1)
#define P_VIDEO_GPU1    (DEV_VIDEO_BASE + 2)
#define P_VIDEO_GPU2    (DEV_VIDEO_BASE + 3)
#define P_VIDEO_GPU3    (DEV_VIDEO_BASE + 4)
#define P_VIDEO_GPU4    (DEV_VIDEO_BASE + 5)
#define P_VIDEO_GPU5    (DEV_VIDEO_BASE + 6)
#define P_VIDEO_GPU6    (DEV_VIDEO_BASE + 7)
#define P_VIDEO_GPU7    (DEV_VIDEO_BASE + 8)
#define P_VIDEO_GPU8    (DEV_VIDEO_BASE + 9)
#define P_VIDEO_GPU9    (DEV_VIDEO_BASE + 10)
#define P_VIDEO_GPU10   (DEV_VIDEO_BASE + 11)
#define P_VIDEO_CUR_X   (DEV_VIDEO_BASE + 12)
#define P_VIDEO_CUR_Y   (DEV_VIDEO_BASE + 13)
#define P_VIDEO_CSR     (DEV_VIDEO_BASE + 14)
#define P_VIDEO_ERROR   (DEV_VIDEO_BASE + 15)

// COMMANDS

enum VideoCommand {
    COMMAND_NOP,
    COMMAND_END_DRAWING,
    COMMAND_BEGIN_DRAWING,

    // Immediate commands
    COMMAND_SYS_INFO,
    COMMAND_BUFFER_INFO,

    // Queued commands
    COMMAND_CLEAR,
    COMMAND_SET_MODE,
    COMMAND_SET_ADDR,
    COMMAND_LOAD,
    COMMAND_BLIT,
    COMMAND_STRETCH_BLIT,
    COMMAND_PATTERN_FILL,

    // GPU commands (queued)
    COMMAND_DRAW_RECT,
    COMMAND_DRAW_LINE,
    COMMAND_DRAW_CIRCLE,
    COMMAND_DRAW_TRI,
    COMMAND_DRAW_BATCH,
    COMMAND_FILL_SPAN,
};

// clang-format off

// The Font type in raylib does not come with a fixed witdh. We are always using monospaced fonts,
// and are using the with of 'M' as the maximum witdth of a character. This is not fool proof.
#define VIDEO_GET_FONT_W(fs)                                                           \
    (int)MeasureTextEx((fs)[VIDEO_FONT_BASE_CP0], "M",                    \
                       (float)(fs)[VIDEO_FONT_BASE_CP0].baseSize, 0.0f).x

// clang-format on

#define ASSEMBLE_CSR(v)                                                 \
    (((v)->queue_full << 7) | 0 | (v)->rop | ((v)->cursor_blink << 4) | \
     ((v)->cursor_enable << 3) | 0 | (v)->vblank_enable)

u8 Video_ReadHandler(TaleaMachine *m, u16 addr)
{
    DeviceVideo *v = &m->video;

    switch (addr) {
    case P_VIDEO_COMMAND: return v->last_executed_command;
    // TODO: Ensure we do indexes everywhere
    case P_VIDEO_CUR_X: return v->cursor_cell_index % v->textbuffer.w;
    case P_VIDEO_CUR_Y: return v->cursor_cell_index / v->textbuffer.w;
    case P_VIDEO_CSR: {
        v->csr = ASSEMBLE_CSR(v);
        return v->csr;
    }
    case P_VIDEO_ERROR: return v->error;
    default: return m->data_memory[addr];
    }
}

// Computes a table of 512 bytes that map each character to its index in the font texture
void Video_computeFontTranslationTable(DeviceVideo *v, enum VideoFontID id, Font font)
{
    int charH       = font.baseSize;
    int charW       = (int)font.recs[1].width;
    int charsPerRow = font.texture.width / charW;

    for (int i = 0; i < 256; i++) {
        Rectangle rec = GetGlyphAtlasRec(font, i);
        v->renderer.font_translation_tables[id][i] =
            (u8)(rec.x / charW + (rec.y / charH) * charsPerRow);
    }
}

void Video_PrepareFont(DeviceVideo *v, enum VideoFontID id, const char *path)
{
    if (v->fonts[id].glyphCount != 0) {
        UnloadRenderTexture(v->renderer.characters_texture);
        MemFree(v->renderer.charsFake);
    }

    v->fonts[id] = LoadFont(path);

    Font *f = &v->fonts[id];

    Video_computeFontTranslationTable(v, id, *f);

    if (id == VIDEO_FONT_BASE_CP0) {
        // Do this if and only if we're loading the primary font
        // Calculate character sizes. Should be equal for all 4 fonts
        Vector2 em      = MeasureTextEx(*f, "M", (float)f->baseSize, 0.0f);
        v->textbuffer.w = (u8)((float)v->renderer.screen_texture->texture.width / em.x);
        v->textbuffer.h = (u8)((float)v->renderer.screen_texture->texture.height / em.y);

        v->renderer.characters_texture = LoadRenderTexture(v->textbuffer.w, v->textbuffer.h);
        SetTextureFilter(v->renderer.characters_texture.texture, TEXTURE_FILTER_POINT);
        v->renderer.charsFake = MemAlloc((size_t)v->textbuffer.w * v->textbuffer.h * sizeof(Color));
    }

    TALEA_LOG_TRACE("Loaded font '%s' \n", path);
}

void Video_RendererInit(DeviceVideo *v, const char *path)
{
    // clang-format off

    v->renderer.shader          = LoadShader(VERTEX_SHADER, path);
    
    v->renderer.cursor_cell_idx_loc = GetShaderLocation(v->renderer.shader, "cursorIndex");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "cursorIndex", v->renderer.cursor_cell_idx_loc);
    v->renderer.csr_loc   = GetShaderLocation(v->renderer.shader, "csr");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "csr", v->renderer.csr_loc);
    v->renderer.chars_loc       = GetShaderLocation(v->renderer.shader, "characters");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "characters", v->renderer.chars_loc);
    v->renderer.font_locs[VIDEO_FONT_BASE_CP0] = GetShaderLocation(v->renderer.shader, "font_cp0");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "font_cp0", v->renderer.font_locs[VIDEO_FONT_BASE_CP0]);
    v->renderer.font_locs[VIDEO_FONT_BASE_CP1] = GetShaderLocation(v->renderer.shader, "font_cp1");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "font_cp1", v->renderer.font_locs[VIDEO_FONT_BASE_CP1]);
    v->renderer.font_locs[VIDEO_ALT_FONT_CP0]  = GetShaderLocation(v->renderer.shader, "alt_font_cp0");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "alt_font_cp0", v->renderer.font_locs[VIDEO_ALT_FONT_CP0]);
    v->renderer.font_locs[VIDEO_ALT_FONT_CP1]  = GetShaderLocation(v->renderer.shader, "alt_font_cp1");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "alt_font_cp1", v->renderer.font_locs[VIDEO_ALT_FONT_CP1]);
    v->renderer.palette_loc     = GetShaderLocation(v->renderer.shader, "palette");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "palette", v->renderer.palette_loc);
    v->renderer.time_loc        = GetShaderLocation(v->renderer.shader, "uTime");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "uTime", v->renderer.time_loc);
    v->renderer.char_size_loc    = GetShaderLocation(v->renderer.shader, "charSize");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "charSize", v->renderer.char_size_loc);
    v->renderer.texture_size_loc = GetShaderLocation(v->renderer.shader, "mainTextureSize");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "mainTextureSize", v->renderer.texture_size_loc);
    TALEA_LOG_ERROR("Shader location for texture0: %d\n",  GetShaderLocation(v->renderer.shader, "texture0"));

    SetShaderValue(v->renderer.shader, v->renderer.cursor_cell_idx_loc, &v->cursor_cell_index, SHADER_UNIFORM_INT);
    SetShaderValue(v->renderer.shader, v->renderer.csr_loc, &v->csr, SHADER_UNIFORM_INT);
    SetShaderValueV(v->renderer.shader, v->renderer.palette_loc, v->renderer.shaderPalette, SHADER_UNIFORM_IVEC4, 256);

    // clang-format on
}

static void Video_ClearScreen(TaleaMachine *m)
{
    memset(&m->main_memory[m->video.framebuffer.addr], 0,
           (size_t)m->video.framebuffer.h * (size_t)m->video.framebuffer.w *
               m->video.framebuffer.stride);
};

static inline void TextMode_AssembleCharacters(TaleaMachine *m)
{
    DeviceVideo *v = &m->video;

    for (size_t i = 0; i < v->textbuffer.h; i++) {
        for (size_t j = 0, k = 0; j < (size_t)v->textbuffer.w * v->textbuffer.stride;
             j += v->textbuffer.stride, k++) {
            //TALEA_LOG_TRACE("w: %d h: %d j: %d k: %d\n", v->textbuffer.w, v->textbuffer.h, j, k);
            size_t cell_addr =
                v->textbuffer.addr + ((size_t)v->textbuffer.w * v->textbuffer.stride * i) + j;

            if (v->mode == VIDEO_MODE_TEXT_COLOR || v->mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
                u8 codepage = m->main_memory[cell_addr + 3] & TEXTMODE_ATT_CODEPAGE;
                u8 alt      = m->main_memory[cell_addr + 3] & TEXTMODE_ATT_ALT_FONT;

                // TALEA_LOG_TRACE("cp: %d, font: %d\n", codepage, alt);
                //  NOTE: the codepage and alt bits are designed so its sum = fontID:
                //  codepage 0 + alt 0 = 0 + 0 base font cp 0 (id 0)
                //  codepage 1 + alt 0 = 1 + 0 base font cp 1 (id 1)
                //  codepage 0 + alt 1 = 0 + 2 alt font cp 0 (id 2)
                //  codepage 1 + alt 1 = 1 + 2 alt font cp 1 (id 3)
                u8 character  = v->renderer.font_translation_tables[0][m->main_memory[cell_addr]];
                u8 fg         = m->main_memory[cell_addr + 1];
                u8 bg         = m->main_memory[cell_addr + 2];
                u8 attributes = m->main_memory[cell_addr + 3];
                v->renderer.charsFake[(v->textbuffer.w * i) + k] = (Color){
                    character,
                    fg,         // foreground (index)
                    bg,         // background (index)
                    attributes, // unpacking in shader
                };
            } else {
                u8 character =
                    v->renderer
                        .font_translation_tables[VIDEO_FONT_BASE_CP0][m->main_memory[cell_addr]];
                v->renderer.charsFake[(v->textbuffer.w * i) + k] = (Color){ character, 0, 0, 0 };
            }
        }
    }
}

// BUG: now in rich text mode the tty does not work as expected

static inline void Framebuffer_Update(TaleaMachine *m)
{
    UpdateTexture(m->video.renderer.framebuffer.texture,
                  &m->main_memory[m->video.framebuffer.addr]);
    DrawTexturePro(m->video.renderer.framebuffer.texture,
                   (Rectangle){ 0.0, 0.0f, (float)m->video.renderer.framebuffer.texture.width,
                                (float)m->video.renderer.framebuffer.texture.height },
                   (Rectangle){ 0.0, 0.0f, (float)m->video.renderer.screen_texture->texture.width,
                                (float)m->video.renderer.screen_texture->texture.height },
                   (Vector2){ 0, 0 }, 0.0f, WHITE);
}

static inline void TextMode_Render(TaleaMachine *m)
{
    UpdateTexture(m->video.renderer.characters_texture.texture, m->video.renderer.charsFake);

    float charSize[2]       = { VIDEO_GET_FONT_W(m->video.fonts),
                                (float)m->video.fonts[VIDEO_FONT_BASE_CP0].baseSize };
    int   textureSize[2]    = { m->video.renderer.screen_texture->texture.width,
                                m->video.renderer.screen_texture->texture.height };
    int   cursor_cell_index = m->video.cursor_cell_index;
    int   cursorCsr = m->video.csr = ASSEMBLE_CSR(&m->video);

    BeginShaderMode(m->video.renderer.shader);

    SetShaderValue(m->video.renderer.shader, m->video.renderer.cursor_cell_idx_loc,
                   &cursor_cell_index, SHADER_UNIFORM_INT);
    SetShaderValue(m->video.renderer.shader, m->video.renderer.csr_loc, &cursorCsr,
                   SHADER_UNIFORM_INT);
    SetShaderValue(m->video.renderer.shader, m->video.renderer.base_color_loc,
                   &m->video.renderer.foreground_color, SHADER_UNIFORM_IVEC4);
    SetShaderValue(m->video.renderer.shader, m->video.renderer.char_size_loc, &charSize,
                   SHADER_UNIFORM_VEC2);
    SetShaderValue(m->video.renderer.shader, m->video.renderer.texture_size_loc, &textureSize,
                   SHADER_UNIFORM_IVEC2);
    SetShaderValueTexture(m->video.renderer.shader,
                          m->video.renderer.font_locs[VIDEO_FONT_BASE_CP0],
                          m->video.fonts[VIDEO_FONT_BASE_CP0].texture);
    SetShaderValueTexture(m->video.renderer.shader,
                          m->video.renderer.font_locs[VIDEO_FONT_BASE_CP1],
                          m->video.fonts[VIDEO_FONT_BASE_CP1].texture);
    SetShaderValueTexture(m->video.renderer.shader, m->video.renderer.font_locs[VIDEO_ALT_FONT_CP0],
                          m->video.fonts[VIDEO_ALT_FONT_CP0].texture);
    SetShaderValueTexture(m->video.renderer.shader, m->video.renderer.font_locs[VIDEO_ALT_FONT_CP1],
                          m->video.fonts[VIDEO_ALT_FONT_CP1].texture);
    SetShaderValueTexture(m->video.renderer.shader, m->video.renderer.chars_loc,
                          m->video.renderer.characters_texture.texture);

    if (!IsShaderValid(m->video.renderer.shader))
        TALEA_LOG_ERROR("On UpdateVideo: Shader is not valid\n");

    // Drawing BLANK texture, all magic happens on shader
    DrawTexture(m->video.renderer.screen_texture->texture, 0, 0, WHITE);
    EndShaderMode(); // Disable our custom shader, return to default shader
}

static void Video_ExecuteCommands(TaleaMachine *m)
{
    DeviceVideo *v = &m->video;
    return;
}

void Video_Update(TaleaMachine *m)
{
    Video_ExecuteCommands(m);

    if (m->video.mode == VIDEO_MODE_TEXT_MONO || m->video.mode == VIDEO_MODE_TEXT_COLOR ||
        m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        TextMode_AssembleCharacters(m);
    }

    BeginTextureMode(*m->video.renderer.screen_texture);
    ClearBackground(BLANK);

    if (m->video.mode == VIDEO_MODE_GRAPHIC || m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        Framebuffer_Update(m);
    }

    if (m->video.mode == VIDEO_MODE_TEXT_MONO || m->video.mode == VIDEO_MODE_TEXT_COLOR ||
        m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        TextMode_Render(m);
    }

    EndTextureMode();

    if (m->video.vblank_enable)
        Machine_RaiseInterrupt(m, INT_VIDEO_REFRESH, PRIORITY_VBLANK_INTERRUPT);
}

static void Video_PushCommandQueue(TaleaMachine *m, u8 op, struct VideoCmd *cmd)
{
    DeviceVideo *v = &m->video;

    u8 next_head = (v->cmd_queue.head + 1) % VIDEO_CMD_QUEUE_SIZE;
    if (next_head == v->cmd_queue.tail) {
        v->queue_full = true;
        v->error      = VIDEO_ERROR_QUEUE_FULL;
        v->csr        = ASSEMBLE_CSR(v);
        return;
    }

    v->cmd_queue.cmd[v->cmd_queue.head].op   = op;
    v->cmd_queue.cmd[v->cmd_queue.head].argc = cmd->argc;
    memcpy(v->cmd_queue.cmd[v->cmd_queue.head].args, cmd->args, VIDEO_CMD_MAX_ARGS);

    v->cmd_queue.head = next_head;
}

static enum VideoError Video_ProcessCommand(TaleaMachine *m, u8 value)
{
    // TODO: document
    DeviceVideo *v = &m->video;

    switch (value) {
    case COMMAND_END_DRAWING: v->is_drawing = false; break;
    case COMMAND_BEGIN_DRAWING: v->is_drawing = true; break;

    // Immediate commands
    case COMMAND_SYS_INFO:
        // character w, h in pixels
        m->data_memory[P_VIDEO_GPU0] = v->fonts[VIDEO_FONT_BASE_CP0].baseSize;
        m->data_memory[P_VIDEO_GPU1] = VIDEO_GET_FONT_W(v->fonts);
        // gives important info on the mode
        m->data_memory[P_VIDEO_GPU2] = v->mode;
        m->data_memory[P_VIDEO_GPU3] = v->textbuffer.stride;
        m->data_memory[P_VIDEO_GPU4] = v->textbuffer.w;
        m->data_memory[P_VIDEO_GPU5] = v->textbuffer.h;
        // gives important info on the screen
        m->data_memory[P_VIDEO_GPU6] = v->framebuffer.w >> 8;
        m->data_memory[P_VIDEO_GPU7] = v->framebuffer.w;
        m->data_memory[P_VIDEO_GPU8] = v->framebuffer.h >> 8;
        m->data_memory[P_VIDEO_GPU9] = v->framebuffer.h;
        v->last_executed_command     = value;
        break;

    case COMMAND_BUFFER_INFO:
        m->data_memory[P_VIDEO_GPU0] = v->textbuffer.addr >> 24;
        m->data_memory[P_VIDEO_GPU1] = v->textbuffer.addr >> 16;
        m->data_memory[P_VIDEO_GPU2] = v->textbuffer.addr >> 8;
        m->data_memory[P_VIDEO_GPU3] = v->textbuffer.addr;

        m->data_memory[P_VIDEO_GPU4] = v->framebuffer.addr >> 24;
        m->data_memory[P_VIDEO_GPU5] = v->framebuffer.addr >> 16;
        m->data_memory[P_VIDEO_GPU6] = v->framebuffer.addr >> 8;
        m->data_memory[P_VIDEO_GPU7] = v->framebuffer.addr;

        m->data_memory[P_VIDEO_GPU8]  = v->spritebuffer.addr >> 16;
        m->data_memory[P_VIDEO_GPU9]  = v->spritebuffer.addr >> 8;
        m->data_memory[P_VIDEO_GPU10] = v->spritebuffer.addr;
        v->last_executed_command      = value;
        break;

    // Queued commands
    default: Video_PushCommandQueue(m, value, &v->current_cmd);
    }

    return VIDEO_NO_ERROR;
}

void Video_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    DeviceVideo *v = &m->video;
    switch (addr) {
    case P_VIDEO_COMMAND: {
        v->error = Video_ProcessCommand(m, value);
        break;
    }
    case P_VIDEO_CUR_X: {
        // We calculate the index
        u8 y                 = v->cursor_cell_index / v->textbuffer.w;
        v->cursor_cell_index = (y * v->textbuffer.w) + value;
        break;
    }
    case P_VIDEO_CUR_Y: {
        u8 x                 = v->cursor_cell_index % v->textbuffer.w;
        v->cursor_cell_index = (value * v->textbuffer.w) + x;
        break;
    }
    case P_VIDEO_CSR: {
        v->vblank_enable = value & VIDEO_VBLANK_EN;
        v->cursor_enable = value & VIDEO_CURSOR_EN;
        v->cursor_blink  = value & VIDEO_CURSOR_BLINK;
        v->rop           = value & (VIDEO_ROP1 | VIDEO_ROP0);

        v->csr = ASSEMBLE_CSR(v);

        if (VIDEO_RESET_REGS) {
            memset(&m->data_memory[P_VIDEO_GPU0], 0, 11);
        }

        break;
    }
    case P_VIDEO_GPU7: {
        if (v->is_drawing) {
            v->current_cmd.args[v->current_cmd.argc] = (u64)m->data_memory[P_VIDEO_GPU0] << 56;
            v->current_cmd.args[v->current_cmd.argc] |= (u64)m->data_memory[P_VIDEO_GPU1] << 48;
            v->current_cmd.args[v->current_cmd.argc] |= (u64)m->data_memory[P_VIDEO_GPU2] << 40;
            v->current_cmd.args[v->current_cmd.argc] |= (u64)m->data_memory[P_VIDEO_GPU3] << 32;
            v->current_cmd.args[v->current_cmd.argc] |= (u64)m->data_memory[P_VIDEO_GPU4] << 24;
            v->current_cmd.args[v->current_cmd.argc] |= (u64)m->data_memory[P_VIDEO_GPU5] << 16;
            v->current_cmd.args[v->current_cmd.argc] |= (u64)m->data_memory[P_VIDEO_GPU6] << 8;
            v->current_cmd.args[v->current_cmd.argc] |= (u64)m->data_memory[P_VIDEO_GPU7];
            memset(&m->data_memory[P_VIDEO_GPU0], 0, 8);
            v->current_cmd.argc++;
        }

        break;
    }
    case P_VIDEO_ERROR: break;
    default: m->data_memory[addr] = value;
    }
}

void Video_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    if (!is_restart) m->video = (DeviceVideo){ 0 };

    struct FrontendState *state = Frontend_GetState();

    m->video = (DeviceVideo){
        .mode                  = VIDEO_MODE_TEXT_COLOR,
        .error                 = VIDEO_NO_ERROR,
        .rop                   = VIDEO_CONFIG_ROP_COPY,
        .last_executed_command = COMMAND_NOP,

        .is_drawing    = false,
        .vblank_enable = false,
        .cursor_enable = true,
        .cursor_blink  = true,
        .queue_full    = false,

        .fonts = { 0 },

        .framebuffer.addr   = TALEA_FRAMEBUFFER_ADDR,
        .framebuffer.w      = TALEA_SCREEN_WIDTH,
        .framebuffer.h      = TALEA_SCREEN_WIDTH,
        .framebuffer.stride = 1,
        .textbuffer.addr    = TALEA_CHARBUFFER_ADDR,
        .textbuffer.w       = 80,
        .textbuffer.h       = 30,
        .textbuffer.stride  = 4,
        .cursor_cell_index  = 0,
        .renderer =
            (VideoRenderer){
                .framebuffer      = LoadRenderTexture(TALEA_SCREEN_WIDTH, TALEA_SCREEN_HEIGHT),
                .background_color = BLACK,
                .foreground_color = GOLD,
                .screen_texture   = &state->window.screenTexture,
            },
    };

    memcpy(&m->video.renderer.shaderPalette, Palette_Default_Aurora, sizeof(u32) * (256 * 4));

    Video_PrepareFont(&m->video, VIDEO_FONT_BASE_CP0,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/cp0.fnt"));
    Video_PrepareFont(&m->video, VIDEO_FONT_BASE_CP1,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/cp1.fnt"));
    Video_PrepareFont(&m->video, VIDEO_ALT_FONT_CP0,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/alt_cp0.fnt"));
    Video_PrepareFont(&m->video, VIDEO_ALT_FONT_CP1,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/alt_cp1.fnt"));

    Video_ClearScreen(m);
    Video_RendererInit(&m->video, SHADERS_PATH("mode_text_color.fs"));
}
