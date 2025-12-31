#define PALETTE_DECLARATION
#include "core/bus.h"
#include "frontend/frontend.h"
#include "talea.h"
#include <string.h>

// PORTS

#define P_VIDEO_COMMAND (DEV_VIDEO_BASE + 0)
#define P_VIDEO_DATAH   (DEV_VIDEO_BASE + 1)
#define P_VIDEO_DATAM   (DEV_VIDEO_BASE + 2)
#define P_VIDEO_DATAL   (DEV_VIDEO_BASE + 3)
#define P_VIDEO_GPU0    (DEV_VIDEO_BASE + 4)
#define P_VIDEO_GPU1    (DEV_VIDEO_BASE + 5)
#define P_VIDEO_GPU2    (DEV_VIDEO_BASE + 6)
#define P_VIDEO_GPU3    (DEV_VIDEO_BASE + 7)
#define P_VIDEO_GPU4    (DEV_VIDEO_BASE + 8)
#define P_VIDEO_GPU5    (DEV_VIDEO_BASE + 9)
#define P_VIDEO_GPU6    (DEV_VIDEO_BASE + 10)
#define P_VIDEO_GPU7    (DEV_VIDEO_BASE + 11)
#define P_VIDEO_STATUS0 (DEV_VIDEO_BASE + 12)
#define P_VIDEO_STATUS1 (DEV_VIDEO_BASE + 13)
#define P_VIDEO_STATUS2 (DEV_VIDEO_BASE + 14)
#define P_VIDEO_STATUS3 (DEV_VIDEO_BASE + 15)

// COMMANDS

enum VideoCommand {
    COMMAND_NOP = 0,
    COMMAND_CLEAR,
    COMMAND_SETMODE,
    COMMAND_SETFONT,
    COMMAND_BLIT,
    COMMAND_SETFB,
    COMMAND_SETVBLANK,
    COMMAND_LOADFONT,
    COMMAND_LOADPALETTE,
    COMMAND_SETBGCOLOR,
    COMMAND_SETFGCOLOR,
    COMMAND_CLEARREGS,

    // TODO: document
    COMMAND_SETFB_SZ,
    COMMAND_SET_BPP,
    COMMAND_SETCB,
    COMMAND_SETCB_SZ,
    COMMAND_SET_BPC,
    COMMAND_FONTINFO,
    COMMAND_MODE_INFO,
    COMMAND_SCREEN_INFO,

    COMMAND_CURSOR_SETPOS,
    COMMAND_CURSOR_SETMODE,
    COMMAND_CURSOR_GETPOS,
};

u8 Video_ReadHandler(TaleaMachine *m, u16 addr)
{
    return m->data_memory[addr];
}

// Computes a table of 256 bytes that map each character to tis index in the
// font texture
void Video_computeFontTranslationTable(DeviceVideo *v, Font font)
{
    int charH       = font.baseSize;
    int charW       = (int)font.recs[1].width;
    int charsPerRow = font.texture.width / charW;

    for (int i = 0; i < 256; i++) {
        Rectangle rec                       = GetGlyphAtlasRec(font, i);
        v->renderer.fontTranslationTable[i] = (u8)(rec.x / charW + (rec.y / charH) * charsPerRow);
    }
}

void VideoNextFont(DeviceVideo *v, const char *path)
{
    if (v->fonts[v->next_font].glyphCount != 0) {
        UnloadRenderTexture(v->renderer.characters_texture);
        MemFree(v->renderer.charsFake);
    }

    v->fonts[v->next_font] = LoadFont(path);

    v->font         = &v->fonts[v->next_font];
    v->current_font = v->next_font;
    Vector2 em      = MeasureTextEx(*v->font, "M", (float)v->font->baseSize, 0.0f);
    v->charbuffer_w = (u8)((float)v->renderer.screen_texture->texture.width / em.x);
    v->charbuffer_h = (u8)((float)v->renderer.screen_texture->texture.height / em.y);
    Video_computeFontTranslationTable(v, *v->font);

    if (v->fonts[++v->next_font].glyphCount != 0) {
        UnloadRenderTexture(v->renderer.characters_texture);
        MemFree(v->renderer.charsFake);
    }

    v->renderer.characters_texture = LoadRenderTexture(v->charbuffer_w, v->charbuffer_h);
    SetTextureFilter(v->renderer.characters_texture.texture, TEXTURE_FILTER_POINT);
    v->renderer.charsFake = MemAlloc((size_t)v->charbuffer_w * v->charbuffer_h * sizeof(Color));
    TALEA_LOG_TRACE("CB dimensions: %dx%d chars\n", v->charbuffer_w, v->charbuffer_h);
    TALEA_LOG_TRACE("Char dimensions: %fx%f chars\n", em.x, em.y);
}

void VideoChangeFont(DeviceVideo *v, u8 fontIndex)
{
    if (v->fonts[fontIndex].glyphCount != 0) {
        if (v->font->glyphCount != 0) {
            UnloadRenderTexture(v->renderer.characters_texture);
            MemFree(v->renderer.charsFake);
        }

        v->font         = &v->fonts[fontIndex];
        v->next_font    = fontIndex + 1;
        v->charbuffer_w = (u8)(v->renderer.screen_texture->texture.width /
                               MeasureTextEx(*v->font, "M", (float)v->font->baseSize, 0.0f).x);
        v->charbuffer_h = (u8)(v->renderer.screen_texture->texture.height / v->font->baseSize);
        Video_computeFontTranslationTable(v, *v->font);

        v->renderer.characters_texture = LoadRenderTexture(v->charbuffer_w, v->charbuffer_h);
        SetTextureFilter(v->renderer.characters_texture.texture, TEXTURE_FILTER_POINT);
        v->renderer.charsFake = MemAlloc((size_t)v->charbuffer_w * v->charbuffer_h * sizeof(Color));
        v->current_font       = fontIndex;
        printf("CB dimensions: %dx%d chars\n", v->charbuffer_w, v->charbuffer_h);
    }
}

void Video_RendererInit(DeviceVideo *v, const char *path)
{
    v->renderer.shader          = LoadShader(VERTEX_SHADER, path);
    v->renderer.cursorIndex_loc = GetShaderLocation(v->renderer.shader, "cursorIndex");
    v->renderer.cursorCsr_loc   = GetShaderLocation(v->renderer.shader, "cursorCsr");
    v->renderer.baseColor_loc   = GetShaderLocation(v->renderer.shader, "baseColor");
    v->renderer.chars_loc       = GetShaderLocation(v->renderer.shader, "characters");
    v->renderer.font_loc        = GetShaderLocation(v->renderer.shader, "font");
    v->renderer.palette_loc     = GetShaderLocation(v->renderer.shader, "palette");
    v->renderer.palettebg_loc   = GetShaderLocation(v->renderer.shader, "paletteBg");
    v->renderer.time_loc        = GetShaderLocation(v->renderer.shader, "uTime");
    v->renderer.charSize_loc    = GetShaderLocation(v->renderer.shader, "charSize");
    v->renderer.textureSize_loc = GetShaderLocation(v->renderer.shader, "mainTextureSize");

    SetShaderValue(v->renderer.shader, v->renderer.cursorIndex_loc, &v->cursorIndex,
                   SHADER_UNIFORM_INT);
    SetShaderValue(v->renderer.shader, v->renderer.cursorCsr_loc, &v->cursorCSR,
                   SHADER_UNIFORM_INT);
    SetShaderValue(v->renderer.shader, v->renderer.baseColor_loc, &v->renderer.foreground_color,
                   SHADER_UNIFORM_IVEC4);
    SetShaderValueV(v->renderer.shader, v->renderer.palette_loc, v->renderer.shaderPalette,
                    SHADER_UNIFORM_IVEC4, 16);
    SetShaderValueV(v->renderer.shader, v->renderer.palettebg_loc, v->renderer.shaderPaletteBg,
                    SHADER_UNIFORM_IVEC4, 8);
}

static void ClearScreen(TaleaMachine *m)
{
    memset(&m->main_memory[m->video.framebuffer_addr], 0,
           (size_t)m->video.framebuffer_h * (size_t)m->video.framebuffer_w * m->video.bpp);
    memset(&m->main_memory[m->video.charbuffer_addr], ' ',
           (size_t)m->video.charbuffer_h * (size_t)m->video.charbuffer_w * m->video.bpc);
};

void Video_Update(TaleaMachine *m)
{
    if (m->video.mode == VIDEO_MODE_TEXT_MONO || m->video.mode == VIDEO_MODE_TEXT_COLOR ||
        m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        for (size_t i = 0; i < m->video.charbuffer_h; i++) {
            for (size_t j = 0, k = 0; j < (size_t)m->video.charbuffer_w * m->video.bpc;
                 j += m->video.bpc, k++) {
                u8 character =
                    m->video.renderer.fontTranslationTable
                        [m->main_memory[m->video.charbuffer_addr +
                                        ((size_t)m->video.charbuffer_w * m->video.bpc * i) + j]];

                if (m->video.mode == VIDEO_MODE_TEXT_COLOR ||
                    m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
                    u8 rawColor =
                        m->main_memory[m->video.charbuffer_addr +
                                       ((size_t)m->video.charbuffer_w * m->video.bpc * i) + j + 1];
                    m->video.renderer.charsFake[(m->video.charbuffer_w * i) + k] = (Color){
                        character,
                        0,        // reserved
                        0,        // reserved
                        rawColor, // unpacking in shader
                    };
                } else {
                    m->video.renderer.charsFake[(m->video.charbuffer_w * i) + k] =
                        (Color){ character, 0, 0, 0 };
                }
            }
        }
    }

    BeginTextureMode(*m->video.renderer.screen_texture);
    ClearBackground(BLANK);

    if (m->video.mode == VIDEO_MODE_GRAPHIC || m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        UpdateTexture(m->video.renderer.framebuffer.texture,
                      &m->main_memory[m->video.framebuffer_addr]);
        DrawTexturePro(m->video.renderer.framebuffer.texture,
                       (Rectangle){ 0.0, 0.0f, (float)m->video.renderer.framebuffer.texture.width,
                                    (float)m->video.renderer.framebuffer.texture.height },
                       (Rectangle){ 0.0, 0.0f,
                                    (float)m->video.renderer.screen_texture->texture.width,
                                    (float)m->video.renderer.screen_texture->texture.height },
                       (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    if (m->video.mode == VIDEO_MODE_TEXT_MONO || m->video.mode == VIDEO_MODE_TEXT_COLOR ||
        m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        // TALEA_LOG_TRACE("Blitting characters by way of a shader\n");

        UpdateTexture(m->video.renderer.characters_texture.texture, m->video.renderer.charsFake);

        float charSize[2] = {
            MeasureTextEx(*m->video.font, "M", (float)m->video.font->baseSize, 0.0f).x,
            (float)m->video.font->baseSize
        };
        int textureSize[2] = { m->video.renderer.screen_texture->texture.width,
                               m->video.renderer.screen_texture->texture.height };
        int cursorIndex    = m->video.cursorIndex;
        int cursorCsr      = m->video.cursorCSR;
        BeginShaderMode(m->video.renderer.shader);

        SetShaderValue(m->video.renderer.shader, m->video.renderer.cursorIndex_loc, &cursorIndex,
                       SHADER_UNIFORM_INT);
        SetShaderValue(m->video.renderer.shader, m->video.renderer.cursorCsr_loc, &cursorCsr,
                       SHADER_UNIFORM_INT);
        SetShaderValue(m->video.renderer.shader, m->video.renderer.baseColor_loc,
                       &m->video.renderer.foreground_color, SHADER_UNIFORM_IVEC4);
        SetShaderValue(m->video.renderer.shader, m->video.renderer.charSize_loc, &charSize,
                       SHADER_UNIFORM_VEC2);
        SetShaderValue(m->video.renderer.shader, m->video.renderer.textureSize_loc, &textureSize,
                       SHADER_UNIFORM_IVEC2);
        SetShaderValueTexture(m->video.renderer.shader, m->video.renderer.font_loc,
                              m->video.font->texture);
        SetShaderValueTexture(m->video.renderer.shader, m->video.renderer.chars_loc,
                              m->video.renderer.characters_texture.texture);
        if (!IsShaderValid(m->video.renderer.shader))
            TALEA_LOG_ERROR("On UpdateVideo: Shader is not valid\n");
        DrawTexture(m->video.renderer.screen_texture->texture, 0, 0,
                    WHITE); // Drawing BLANK texture, all magic happens on
                            // shader
        EndShaderMode();    // Disable our custom shader, return to default shader
    }
    EndTextureMode();

    if (m->video.vblank_enable)
        Machine_RaiseInterrupt(m, INT_VIDEO_REFRESH, PRIORITY_VBLANK_INTERRUPT);
}

void Video_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    if (addr == P_VIDEO_COMMAND) {
        switch (value) {
        case COMMAND_NOP: break;
        case COMMAND_CLEAR: ClearScreen(m); break;
        case COMMAND_SETMODE:
            printf("[VIDEO] Changing to mode %d\n",
                   m->data_memory[P_VIDEO_DATAH]); // TODO: maybe automatically
                                                   // clear the screen
            m->video.mode = m->data_memory[P_VIDEO_DATAH];

            if (m->video.mode == VIDEO_MODE_TEXT_MONO) {
                UnloadShader(m->video.renderer.shader);
                m->video.bpc = 1;
                Video_RendererInit(&m->video, SHADERS_PATH("mode_text_mono.fs"));
            } else {
                UnloadShader(m->video.renderer.shader);
                m->video.bpc = 2;
                Video_RendererInit(&m->video, SHADERS_PATH("mode_text_color.fs"));
            };
            break;
        case COMMAND_SETFONT: m->video.font = &m->video.fonts[P_VIDEO_DATAH]; break;
        case COMMAND_BLIT:
            // not implemented
            break;
        case COMMAND_SETFB:
            m->video.framebuffer_addr = (u32)m->data_memory[P_VIDEO_DATAH] << 16 |
                                        (u32)m->data_memory[P_VIDEO_DATAM] << 8 |
                                        m->data_memory[P_VIDEO_DATAL];
            break;
        case COMMAND_SETFB_SZ: {
            u16 w = (u16)m->data_memory[P_VIDEO_DATAH] << 8 | m->data_memory[P_VIDEO_DATAM];
            u16 h = (u16)m->data_memory[P_VIDEO_GPU0] << 8 | m->data_memory[P_VIDEO_GPU1];
            m->video.framebuffer_w = (w) ? w : m->video.framebuffer_w;
            m->video.framebuffer_h = (h) ? h : m->video.framebuffer_h;
            break;
        }
        case COMMAND_SET_BPP: m->video.bpp = m->data_memory[P_VIDEO_DATAH]; break;
        case COMMAND_SETCB:
            m->video.charbuffer_addr = (u32)m->data_memory[P_VIDEO_DATAH] << 16 |
                                       (u32)m->data_memory[P_VIDEO_DATAM] << 8 |
                                       m->data_memory[P_VIDEO_DATAL];
            break;
        case COMMAND_SETCB_SZ: {
            u8 w                  = m->data_memory[P_VIDEO_DATAH];
            u8 h                  = m->data_memory[P_VIDEO_DATAM];
            m->video.charbuffer_w = (w) ? w : m->video.charbuffer_w;
            m->video.charbuffer_h = (h) ? h : m->video.charbuffer_h;
            break;
        }
        case COMMAND_SET_BPC: m->video.bpc = m->data_memory[P_VIDEO_DATAH]; break;
        case COMMAND_SETVBLANK: m->video.vblank_enable = m->data_memory[P_VIDEO_DATAH]; break;
        case COMMAND_LOADFONT: {
            u8 *fontData = &m->main_memory[(u32)m->data_memory[P_VIDEO_DATAH] << 16 |
                                           (u32)m->data_memory[P_VIDEO_DATAM] << 8 |
                                           m->data_memory[P_VIDEO_DATAL]];

            int fontSize = (u32)m->data_memory[P_VIDEO_GPU0] << 24 |
                           (u32)m->data_memory[P_VIDEO_GPU1] << 16 |
                           (u32)m->data_memory[P_VIDEO_GPU2] << 8 | m->data_memory[P_VIDEO_GPU3];

            SaveFileData(VIDEO_FONT_FILE_HACK, fontData, fontSize);
            m->video.fonts[m->video.next_font++] = LoadFont(VIDEO_FONT_FILE_HACK);

            break;
        }
        case COMMAND_LOADPALETTE:
            memcpy(&m->video.renderer.shaderPalette,
                   &m->main_memory[(u32)m->data_memory[P_VIDEO_DATAH] << 16 |
                                   (u32)m->data_memory[P_VIDEO_DATAM] << 8 |
                                   m->data_memory[P_VIDEO_DATAL]],
                   sizeof(m->video.renderer.shaderPalette));
            memcpy(&m->video.renderer.shaderPaletteBg,
                   &m->main_memory[(u32)m->data_memory[P_VIDEO_DATAH] << 16 |
                                   (u32)m->data_memory[P_VIDEO_DATAM] << 8 |
                                   m->data_memory[P_VIDEO_DATAL]],
                   sizeof(m->video.renderer.shaderPaletteBg));
            m->video.renderer.shaderPaletteBg[0] = 0;
            break;
        case COMMAND_SETBGCOLOR:
            m->video.renderer.background_color =
                (Color){ m->data_memory[P_VIDEO_GPU0], m->data_memory[P_VIDEO_GPU1],
                         m->data_memory[P_VIDEO_GPU2], m->data_memory[P_VIDEO_GPU3] };
            break;
        case COMMAND_SETFGCOLOR:
            m->video.renderer.foreground_color =
                (Color){ m->data_memory[P_VIDEO_GPU0], m->data_memory[P_VIDEO_GPU1],
                         m->data_memory[P_VIDEO_GPU2], m->data_memory[P_VIDEO_GPU3] };
            break;
        case COMMAND_CLEARREGS: memset(&m->data_memory[DEV_VIDEO_BASE], 0, 16); break;
        case COMMAND_FONTINFO:
            // gives important info on the font
            m->data_memory[P_VIDEO_STATUS0] = m->video.font->baseSize; // character
                                                                       // height
                                                                       // in
                                                                       // Pixels
            m->data_memory[P_VIDEO_STATUS1] =
                (int)MeasureTextEx(*m->video.font, "M", (float)m->video.font->baseSize, 0.0f)
                    .x;                                           // M size (width for monospace)
            m->data_memory[P_VIDEO_STATUS2] = m->video.next_font; // font index
            m->data_memory[P_VIDEO_STATUS3] = m->video.next_font - 1; // number
                                                                      // of
                                                                      // fonts
                                                                      // loaded
            break;
        case COMMAND_MODE_INFO:
            // gives important info on the mode
            m->data_memory[P_VIDEO_STATUS0] = m->video.mode;
            m->data_memory[P_VIDEO_STATUS1] = m->video.bpc;
            m->data_memory[P_VIDEO_STATUS2] = m->video.bpp;
            m->data_memory[P_VIDEO_STATUS3] = 0;
            break;
        case COMMAND_SCREEN_INFO:
            // gives important info on the screen
            m->data_memory[P_VIDEO_STATUS0] = TALEA_SCREEN_WIDTH >> 8;
            m->data_memory[P_VIDEO_STATUS1] = TALEA_SCREEN_WIDTH;
            m->data_memory[P_VIDEO_STATUS2] = TALEA_SCREEN_HEIGHT >> 8;
            m->data_memory[P_VIDEO_STATUS3] = TALEA_SCREEN_HEIGHT;
            break;
        case COMMAND_CURSOR_SETPOS:
            // sets the cursor position based on DATA_M and DATA_L
            m->video.cursorIndex = m->data_memory[P_VIDEO_DATAM] << 8 |
                                   m->data_memory[P_VIDEO_DATAL];
            break;
        case COMMAND_CURSOR_SETMODE:
            // sets the cursor mode (see CURSOR_CSR)
            m->video.cursorCSR = m->data_memory[P_VIDEO_DATAL];
        case COMMAND_CURSOR_GETPOS:
            // returns the cursor position based on DATA_M and DATA_L
            // and the CSR in DATAH
            m->data_memory[P_VIDEO_DATAH] = m->video.cursorCSR;
            m->data_memory[P_VIDEO_DATAM] = m->video.cursorCSR >>  8;
            m->data_memory[P_VIDEO_DATAL] = m->video.cursorIndex;
            break;
        default:
            m->data_memory[P_VIDEO_STATUS0] = 0xDE;
            m->data_memory[P_VIDEO_STATUS1] = 0xAD;
            m->data_memory[P_VIDEO_STATUS2] = 0xBE;
            m->data_memory[P_VIDEO_STATUS3] = 0xEF;
            break;
        }
    } else {
        m->data_memory[addr] = value;
    }
}

void Video_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    if (!is_restart) m->video = (DeviceVideo){ 0 };

    struct FrontendState *state = Frontend_GetState();

    m->video= (DeviceVideo)
    {
        .mode = VIDEO_MODE_TEXT_MONO, .font = &m->video.fonts[0], .fonts = { 0 },
        .next_font = 0, .vblank_enable = false,
        .framebuffer_addr = TALEA_FRAMEBUFFER_ADDR,
        .framebuffer_w    = TALEA_SCREEN_WIDTH,
        .framebuffer_h    = TALEA_SCREEN_WIDTH,
        .charbuffer_addr  = TALEA_CHARBUFFER_ADDR,
        .bpc              = 1,
        .bpp              = 4,
        .cursorIndex = 0,
        .cursorCSR = CURSOR_ENABLE | CURSOR_BLINK,
        .renderer = (VideoRenderer) {
            .framebuffer = LoadRenderTexture(TALEA_SCREEN_WIDTH, TALEA_SCREEN_HEIGHT),
            .background_color = BLACK,
            .foreground_color = GOLD,
            .screen_texture   = &state->window.screenTexture,
            .shaderPalette = {
                0x00, 0x00, 0x00, 0xff, // BLACK
                0x99, 0x00, 0x00, 0xff, // RED
                0x00, 0x66, 0x00, 0xff, // GREEN
                0xcc, 0xcc, 0x00, 0xff, // GOLD
                0x00, 0x00, 0x99, 0xff, // INDIGO
                0x99, 0x00, 0x99, 0xff, // MAGENTA
                0x00, 0x4c, 0x99, 0xff, // BLUE
                0xe0, 0xe0, 0xe0, 0xff, // BONE
                0x60, 0x60, 0x60, 0xff, // GREY
                0xff, 0x99, 0x33, 0xff, // ORANGE
                0x33, 0xff, 0x33, 0xff, // LIME
                0xff, 0xff, 0x33, 0xff, // YELLOW
                0x66, 0x66, 0xff, 0xff, // VIOLET
                0xff, 0x33, 0xff, 0xff, // PINK
                0x33, 0xff, 0xff, 0xff, // CYAN
                0xff, 0xff, 0xff, 0xff, // WHITE
            },
            .shaderPaletteBg = {
                0x00, 0x00, 0x00, 0x00, // TRANS 
                0x99, 0x00, 0x00, 0xff, // RED
                0x00, 0x66, 0x00, 0xff, // GREEN
                0xcc, 0xcc, 0x00, 0xff, // YELLOW
                0x00, 0x00, 0x99, 0xff, // INDIGO
                0x99, 0x00, 0x99, 0xff, // MAGENTA
                0x00, 0x4c, 0x99, 0xff, // BLUE
                0xe0, 0xe0, 0xe0, 0xff, // WHITE
            },
        },
    };

    VideoNextFont(&m->video, TextFormat("%s%s", FONT_PATH, "default.fnt"));
    VideoNextFont(&m->video, TextFormat("%s%s", FONT_PATH, "cozzete.fnt"));
    VideoNextFont(&m->video, TextFormat("%s%s", FONT_PATH, "spleen8x16.fnt"));
    VideoNextFont(&m->video, TextFormat("%s%s", FONT_PATH, "vt22016.fnt"));
    VideoNextFont(&m->video, TextFormat("%s%s", FONT_PATH, "vt220.fnt"));
    VideoNextFont(&m->video, TextFormat("%s%s", FONT_PATH, config->hardware_font));

    Frontend_SetActiveFont(m->video.current_font);
    ClearScreen(m);
    Video_RendererInit(&m->video, SHADERS_PATH("mode_text_mono.fs"));
}
