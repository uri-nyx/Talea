#include <math.h>
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
    COMMAND_FILL_VSPAN,

    COMMAND_SET_CSR, // queued for use in blits
    COMMAND_BIND_CTX,
};

// clang-format off

// The Font type in raylib does not come with a fixed witdh. We are always using monospaced fonts,
// and are using the with of 'M' as the maximum witdth of a character. This is not fool proof.
#define VIDEO_GET_FONT_W(fs)                                                           \
    (int)MeasureTextEx((fs)[VIDEO_FONT_BASE_CP0], "M",                    \
                       (float)(fs)[VIDEO_FONT_BASE_CP0].baseSize, 0.0f).x

// clang-format on

#define ASSEMBLE_CSR(v)                                                    \
    (((v)->queue_full << 7) | ((v)->rop << 4) | ((v)->cursor_blink << 3) | \
     ((v)->cursor_enable << 2) | 0 | (v)->vblank_enable)

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
    v->renderer.fb_shader          = LoadShader(VERTEX_SHADER, SHADERS_PATH("fb_pixels.fs"));
    
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

    v->renderer.fb_loc = GetShaderLocation(v->renderer.fb_shader, "pixels");
    v->renderer.fb_palette_loc = GetShaderLocation(v->renderer.fb_shader, "palette");
    SetShaderValueV(v->renderer.fb_shader, v->renderer.fb_palette_loc, v->renderer.shaderPalette, SHADER_UNIFORM_IVEC4, 256);

    // clang-format on
}

static inline void TextMode_AssembleCharacters(TaleaMachine *m)
{
    DeviceVideo *v = &m->video;

    for (size_t i = 0; i < v->textbuffer.h; i++) {
        for (size_t j = 0, k = 0; j < (size_t)v->textbuffer.w * v->textbuffer.stride;
             j += v->textbuffer.stride, k++) {
            // TALEA_LOG_TRACE("w: %d h: %d j: %d k: %d\n", v->textbuffer.w, v->textbuffer.h, j, k);
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
                u8 character =
                    v->renderer.font_translation_tables[codepage + alt][m->main_memory[cell_addr]];
                u8 fg                                            = m->main_memory[cell_addr + 1];
                u8 bg                                            = m->main_memory[cell_addr + 2];
                u8 attributes                                    = m->main_memory[cell_addr + 3];
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
    VideoRenderer *r = &m->video.renderer;

    UpdateTexture(r->pixels, &m->main_memory[m->video.framebuffer.addr]);
    BeginShaderMode(r->fb_shader);
    SetShaderValueTexture(r->fb_shader, r->fb_loc, r->pixels);
    DrawTexture(r->pixels, 0, 0, WHITE);
    EndShaderMode();

    DrawTexturePro(r->framebuffer.texture,
                   (Rectangle){ 0.0, 0.0f, (float)r->framebuffer.texture.width,
                                (float)r->framebuffer.texture.height },
                   (Rectangle){ 0.0, 0.0f, (float)r->screen_texture->texture.width,
                                (float)r->screen_texture->texture.height },
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
    DrawTexture(m->video.renderer.screen_texture->texture, 0, 0, BLANK);
    EndShaderMode(); // Disable our custom shader, return to default shader
}

static struct VideoCmd *Video_PopCommandQueue(TaleaMachine *m)
{
    DeviceVideo *v = &m->video;

    if (v->cmd_queue.head != v->cmd_queue.tail) {
        return &v->cmd_queue.cmd[v->cmd_queue.tail++];
    }

    return NULL;
}

static inline u32 toBe32(u32 u)
{
    return (u << 24) | ((u & 0x0000FF00) << 8) | ((u & 0x00FF0000) >> 8) | (u >> 24);
}

static inline void Video_Clear(TaleaMachine *m, DeviceVideo *v, u32 pattern, u8 color)
{
    if (v->mode == VIDEO_MODE_TEXT_MONO) {
        memset(&m->main_memory[v->textbuffer.addr], ' ', v->textbuffer.h * v->textbuffer.w);
    } else {
        u32 *text       = (u32 *)&m->main_memory[v->textbuffer.addr];
        u32  be_pattern = toBe32(pattern);
        for (size_t i = 0; i < v->textbuffer.w * v->textbuffer.h; i++) {
            text[i] = be_pattern;
        }

        memset(&m->main_memory[v->framebuffer.addr], color, v->framebuffer.w * v->framebuffer.h);
    }
}

static inline void Video_SetMode(DeviceVideo *v, u8 mode)
{
    // TODO: implement small font
    if (mode > VIDEO_MODE_TEXT_AND_GRAPHIC) {
        v->error = VIDEO_ERROR_NO_MODE;
        return;
    }

    if (mode == VIDEO_MODE_TEXT_MONO) {
        UnloadShader(v->renderer.shader);
        v->textbuffer.stride = 1;
        Video_RendererInit(v, SHADERS_PATH("mode_text_mono.fs"));
    } else {
        UnloadShader(v->renderer.shader);
        v->textbuffer.stride = 2;
        Video_RendererInit(v, SHADERS_PATH("mode_text_color.fs"));
    }
}

static inline void Video_ApplyROP(DeviceVideo *v, u8 *dest, u8 pixel)
{
    switch (v->rop) {
    case VIDEO_CONFIG_ROP_COPY: (*dest) = pixel; break;
    case VIDEO_CONFIG_ROP_XOR: (*dest) ^= pixel; break;
    case VIDEO_CONFIG_ROP_AND: (*dest) &= pixel; break;
    case VIDEO_CONFIG_ROP_OR: (*dest) |= pixel; break;
    case VIDEO_CONFIG_ROP_NOT: (*dest) = ~(*dest); break;
    case VIDEO_CONFIG_ROP_TRANS:
        if (pixel != v->transparency) {
            (*dest) = pixel;
        }
        break;
    case VIDEO_CONFIG_ROP_AND_NOT: (*dest) &= ~pixel; break;
    case VIDEO_CONFIG_ROP_ADDS: {
        u16 sum = (*dest) + pixel;
        (*dest) = sum > 255 ? 255 : (u8)sum;
    }
    default: break;
    }
}

typedef struct {
    int  a, b, c, d;
    int  xo, yo;
    bool swap;
} RotationMatrix;

static RotationMatrix rot_table[] = {
    [VIDEO_ROT_IDENT]     = { .a = 1, .b = 0, .c = 0, .d = 1, .xo = 0, .yo = 0, .swap = false },
    [VIDEO_ROT_FLIPH]     = { .a = -1, .b = 0, .c = 0, .d = 1, .xo = 1, .yo = 0, .swap = false },
    [VIDEO_ROT_FLIPV]     = { .a = 1, .b = 0, .c = 0, .d = -1, .xo = 0, .yo = 2, .swap = false },
    [VIDEO_ROT_180]       = { .a = -1, .b = 0, .c = 0, .d = -1, .xo = 1, .yo = 2, .swap = false },
    [VIDEO_ROT_90]        = { .a = 0, .b = -1, .c = 1, .d = 0, .xo = 2, .yo = 0, .swap = true },
    [VIDEO_ROT_270]       = { .a = 0, .b = 1, .c = -1, .d = 0, .xo = 0, .yo = 1, .swap = true },
    [VIDEO_ROT_TRANS]     = { .a = 0, .b = 1, .c = 1, .d = 0, .xo = 0, .yo = 0, .swap = true },
    [VIDEO_ROT_ANTITRANS] = { .a = 0, .b = -1, .c = -1, .d = 0, .xo = 2, .yo = 1, .swap = true },
};

static void Video_Blit(TaleaMachine *m, DeviceVideo *vid, struct Buff2D *dest, u8 *src, u16 w,
                       u16 h, i16 x, i16 y, u16 dest_w, u16 dest_h,
                       enum VideoSpriteRotation rotation)
{
    u8 *dest_start = &m->main_memory[dest->addr];
    u8 *dest_end   = dest_start + (dest->w * dest->h * dest->stride);

    u8  *temp_src = src;
    bool is_temp  = false;

    if (src >= dest_start && src < dest_end) {
        // buffers overlap
        temp_src = malloc(w * h * sizeof(u8));
        if (!temp_src) {
            // panic here?
            TALEA_LOG_ERROR("Out of mem blitting\n");
            return;
        }

        memcpy(temp_src, src, w * h * sizeof(u8));
        is_temp = true;
    }

    u16 v_x1 = MAX(0, x);
    u16 v_y1 = MAX(0, y);
    u16 v_x2 = MIN(dest->w, x + dest_w);
    u16 v_y2 = MIN(dest->h, y + dest_h);

    if (v_x1 >= v_x2 || v_y1 >= v_y2) {
        if (is_temp) free(temp_src);
        return;
    }

    RotationMatrix *mat = &rot_table[rotation];

    int xo = mat->xo == 1 ? (int)w - 1 : mat->xo == 2 ? (int)h - 1 : 0;
    int yo = mat->yo == 1 ? (int)w - 1 : mat->yo == 2 ? (int)h - 1 : 0;

    int ew = mat->swap ? h : w;
    int eh = mat->swap ? w : h;

    double ratio_x = (double)dest_w / (double)ew;
    double ratio_y = (double)dest_h / (double)eh;

    TALEA_LOG_TRACE("Blitting with rotation: %d\n", rotation);

    for (size_t v = 0; v < h; v++) {
        for (size_t u = 0; u < w; u++) {
            int tx = (u * mat->a) + (v * mat->b) + xo;
            int ty = (u * mat->c) + (v * mat->d) + yo;

            int base_x = x + (int)(tx * ratio_x);
            int base_y = y + (int)(ty * ratio_y);

            u8 pixel = temp_src[v * w + u];

            for (size_t sy = 0; sy < (int)ceil(ratio_y); sy++) {
                for (size_t sx = 0; sx < (int)ceil(ratio_x); sx++) {
                    int curr_x = base_x + sx;
                    int curr_y = base_y + sy;

                    if (curr_x >= v_x1 && curr_x < v_x2 && curr_y >= v_y1 && curr_y < v_y2) {
                        u8 *target = dest_start + (curr_y * dest->w + curr_x);
                        Video_ApplyROP(vid, target, pixel);
                    }
                }
            }
        }
    }

    if (is_temp) free(temp_src);
}

static void Video_DrawRect(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color, i16 x,
                           i16 y, u16 w, u16 h)
{
    if (x >= dest->w || y >= dest->h) {
        TALEA_LOG_WARNING("Blit, coordinates outside framebuffer\n");
        // TODO: maybe raise an error
        return;
    }

    u16 v_x1 = MAX(0, x);
    u16 v_y1 = MAX(0, y);
    u16 v_x2 = MIN(dest->w, x + w);
    u16 v_y2 = MIN(dest->h, y + h);

    if (v_x1 >= v_x2 || v_y1 >= v_y2) return;

    u8 *fb = &m->main_memory[dest->addr];

    TALEA_LOG_TRACE("ROP: %d\n", v->rop);
    for (size_t row = v_y1; row < v_y2; row++) {
        u8 *rect = fb + (dest->w * row + v_x1) * dest->stride;
        for (size_t col = v_x1; col < v_x2; col++) {
            Video_ApplyROP(v, &rect[col - v_x1], color);
        }
    }
}

static void Video_PatternFill(TaleaMachine *m, DeviceVideo *vid, struct Buff2D *dest, u8 *src, u8 w,
                              u8 h, u8 u_off, u8 v_off, i16 x, i16 y, u16 dest_w, u16 dest_h,
                              enum VideoSpriteRotation rotation)
{
    u8             *dest_start = &m->main_memory[dest->addr];
    RotationMatrix *mat        = &rot_table[rotation];

    u16 v_x1 = MAX(0, x);
    u16 v_y1 = MAX(0, y);
    u16 v_x2 = MIN(dest->w, x + dest_w);
    u16 v_y2 = MIN(dest->h, y + dest_h);

    int xo = mat->xo == 1 ? (int)w - 1 : mat->xo == 2 ? (int)h - 1 : 0;
    int yo = mat->yo == 1 ? (int)w - 1 : mat->yo == 2 ? (int)h - 1 : 0;

    int tile_w = mat->swap ? h : w;
    int tile_h = mat->swap ? w : h;

    for (size_t curr_y = v_y1; curr_y < v_y2; curr_y++) {
        u8 *row = dest_start + (curr_y * dest->w);
        for (size_t curr_x = v_x1; curr_x < v_x2; curr_x++) {
            int local_x = curr_x - x;
            int local_y = curr_y - y;

            int tx = (local_x * mat->a) + (local_y * mat->b) + xo;
            int ty = (local_x * mat->c) + (local_y * mat->d) + yo;

            int u = (tx + u_off) % tile_w;
            int v = (ty + v_off) % tile_h;

            u = u < 0 ? u + tile_w : u;
            v = v < 0 ? v + tile_h : v;

            u8 pixel;

            if (mat->swap) {
                pixel = src[u * w + v];
            } else {
                pixel = src[v * w + u];
            }

            u8 *target = row + curr_x;
            Video_ApplyROP(vid, target, pixel);
        }
    }
}

static inline void Buff2D_SetPixel(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                   i16 x, i16 y)
{
    if (x >= dest->w || y >= dest->h) return;
    if (x < 0 || y < 0) return;
    u8    *surface = &m->main_memory[dest->addr];
    size_t index   = ((y * dest->w) + x) * dest->stride;
    Video_ApplyROP(v, &surface[index], color);
}

static void Video_DrawHorizontalLine(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                     i16 x1, i16 x2, i16 y)
{
    TALEA_LOG_TRACE("Drawing scanline %d from %d to %d, color %x\n", y, x1, x2, color);

    if (y < 0 || y >= dest->h) return;

    size_t start_x = MAX(0, MIN(x1, x2));
    size_t end_x   = MIN(dest->w - 1, MAX(x1, x2));

    if (start_x > end_x) return;

    u8 *row = &m->main_memory[dest->addr + ((y * dest->w))];

    if (v->rop == VIDEO_CONFIG_ROP_COPY) {
        memset(&row[start_x], color, (end_x - start_x) + 1);
    } else {
        for (size_t col = start_x; col < end_x + 1; col++) {
            Video_ApplyROP(v, &row[col], color);
        }
    }
}

static void Video_DrawVerticalLine(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                   i16 x, i16 y1, i16 y2)
{
    TALEA_LOG_TRACE("Drawing vline %d from %d to %d, color %x\n", x, y1, y2, color);

    if (x < 0 || x >= dest->w) return;

    size_t start_y = MAX(0, MIN(y1, y2));
    size_t end_y   = MIN(dest->h - 1, MAX(y1, y2));

    if (start_y > end_y) return;

    u8 *buff = &m->main_memory[dest->addr];

    u8 *current_pixel = buff + (start_y * dest->w + x);
    u16 v_stride      = dest->w;

    for (size_t i = 0; i < (end_y - start_y) + 1; i++, current_pixel += v_stride) {
        Video_ApplyROP(v, current_pixel, color);
    }
}

static void Video_DrawLine(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color, i16 x0,
                           i16 y0, i16 x1, i16 y1)
{
    if (x0 == x1) {
        Video_DrawVerticalLine(m, v, dest, color, x0, y0, y1);
        return;
    }

    if (y0 == y1) {
        Video_DrawHorizontalLine(m, v, dest, color, x0, x1, y0);
    }

    // from http://members.chello.at/easyfilter/bresenham.html
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */
        Buff2D_SetPixel(m, v, dest, color, x0, y0);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        } /* e_xy+e_x > 0 */
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        } /* e_xy+e_y < 0 */
    }
}

static void Video_DrawFilledCircle(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                   i16 xm, i16 ym, i16 r)
{
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (x <= y) {
        Video_DrawHorizontalLine(m, v, dest, color, xm - x, xm + x, ym + y);
        Video_DrawHorizontalLine(m, v, dest, color, xm - x, xm + x, ym - y);
        Video_DrawHorizontalLine(m, v, dest, color, xm - y, xm + y, ym + x);
        Video_DrawHorizontalLine(m, v, dest, color, xm - y, xm + y, ym - x);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y -= 1;
        }
        x += 1;
    }
}

static void Video_DrawCircle(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 mode,
                             u8 color_line, u8 color_fill, i16 xm, i16 ym, i16 r)
{
    // from http://members.chello.at/easyfilter/bresenham.html
    // TODO: implement filling

    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    int last_y = -1;
    int last_x = 1;
    int fill_r = r - 1;

    do {
        Buff2D_SetPixel(m, v, dest, color_line, xm - x, ym + y); /*   I. Quadrant */
        Buff2D_SetPixel(m, v, dest, color_line, xm - y, ym - x); /*  II. Quadrant */
        Buff2D_SetPixel(m, v, dest, color_line, xm + x, ym - y); /* III. Quadrant */
        Buff2D_SetPixel(m, v, dest, color_line, xm + y, ym + x); /*  IV. Quadrant */
        r = err;
        if (r <= y) err += ++y * 2 + 1;           /* e_xy+e_y < 0 */
        if (r > x || err > y) err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);

    if (mode & DRAW_MODE_CIRCLE_FILL) {
        Video_DrawFilledCircle(m, v, dest, color_fill, xm, ym, fill_r);
    }
}

static inline int EdgeFunction(i16 x0, i16 y0, i16 x1, i16 y1, i16 x2, i16 y2)
{
    return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
}

static inline bool isTopLeft(i16 x0, i16 y0, i16 x1, i16 y1)
{
    int  edge_x  = x1 - x0;
    int  edge_y  = y1 - y0;
    bool is_top  = edge_y == 0 && edge_x > 0;
    bool is_left = edge_y > 0;
    return is_left || is_top;
}

static void Video_DrawTri(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color, i16 x0,
                          i16 y0, i16 x1, i16 y1, i16 x2, i16 y2)
{
    int minx = MAX(0, MIN(x0, MIN(x1, x2)));
    int miny = MAX(0, MIN(y0, MIN(y1, y2)));
    int maxx = MIN(dest->w - 1, MAX(x0, MAX(x1, x2)));
    int maxy = MIN(dest->h - 1, MAX(y0, MAX(y1, y2)));

    if (minx > maxx || miny > maxy) return;

    if (EdgeFunction(x0, y0, x1, y1, x2, y2) < 0) {
        i16 temp_x = x1;
        i16 temp_y = y1;
        x1         = x2;
        y1         = y2;
        x2         = temp_x;
        y2         = temp_y;
    }

    int A01 = y0 - y1, B01 = x1 - x0;
    int A12 = y1 - y2, B12 = x2 - x1;
    int A20 = y2 - y0, B20 = x0 - x2;

    int bias0 = isTopLeft(x1, y1, x2, y2) ? 0 : -1;
    int bias1 = isTopLeft(x2, y2, x0, y0) ? 0 : -1;
    int bias2 = isTopLeft(x0, y0, x1, y1) ? 0 : -1;

    int abp = EdgeFunction(x1, y1, x2, y2, minx, miny) + bias0;
    int bcp = EdgeFunction(x2, y2, x0, y0, minx, miny) + bias1;
    int cap = EdgeFunction(x0, y0, x1, y1, minx, miny) + bias2;

    for (size_t py = miny; py < maxy; py++) {
        int w0 = abp;
        int w1 = bcp;
        int w2 = cap;

        for (size_t px = minx; px < maxx; px++) {
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                Buff2D_SetPixel(m, v, dest, color, px, py);
            }

            // One step to the right
            w0 += A12;
            w1 += A20;
            w2 += A01;
        }
        // One row step
        abp += B12;
        bcp += B20;
        cap += B01;
    }
}

static void Video_ExecuteCommand(TaleaMachine *m, DeviceVideo *v, struct VideoCmd *cmd)
{
    switch (cmd->op) {
    case COMMAND_CLEAR: {
        // TODO: Document
        // Takes 1 word on GPU0-3 for the pattern to clear the textbuffer
        // Takes 1 byte on GPU4 for the color to clear the framebuffer
        // If mode is text mono, arguments ignored
        if (cmd->argc > 1) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 1) {
            goto not_enough_args;
            break;
        }

        u32 pattern = cmd->args[0] >> 32;
        u8  color   = cmd->args[0] >> 24;
        Video_Clear(m, v, pattern, color);
    } break;
    case COMMAND_SET_MODE: {
        // TODO: Document
        // Takes 1 byte on GPU0 for the mode number.
        // If mode does not exist
        if (cmd->argc > 1) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 1) {
            goto not_enough_args;
            break;
        }

        u8 mode = cmd->args[0] >> 56;
        Video_SetMode(v, mode);

        break;
    }
    case COMMAND_SET_ADDR: {
        // TODO: Document
        //  Takes 1 word on GPU0-3 for the framebuffer address
        //  Takes 1 word on GPU4-7 for the textbuffer address
        //  If the pattern is >= 0x00_FF_FF_FF, the buffer stays where it is

        if (cmd->argc > 1) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 1) {
            goto not_enough_args;
            break;
        }

        u32 fb = cmd->args[0] >> 32;
        u32 tb = cmd->args[0];

        if (fb < 0xffffff) v->framebuffer.addr = fb;
        if (tb < 0xffffff) v->textbuffer.addr = tb;

        break;
    }
    case COMMAND_LOAD: break; // TODO: implement font loading
    case COMMAND_BLIT: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 sesqui GPU1-3 for the buffer address
        //  Takes 1 half word on GPU4-5 for the buffer width
        //  Takes 1 half word on GPU6-7 for the buffer height
        // Arg 1:
        //  Takes 1 half word on GPU0-1 for the dest x
        //  Takes 1 half word on GPU2-3 for the dest y
        //  Takes 1 byte on GPU4 for the rotation flag

        if (cmd->argc > 2) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 2) {
            goto not_enough_args;
            break;
        }

        u8             ctx  = cmd->args[0] >> 56;
        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u32 src_addr = (cmd->args[0] >> 32) & 0x00ffffff;
        u8 *src      = &m->main_memory[src_addr];

        u16 w = cmd->args[0] >> 16;
        u16 h = cmd->args[0];

        u16 x = cmd->args[1] >> 48;
        u16 y = cmd->args[1] >> 32;

        enum VideoSpriteRotation rotation = (cmd->args[1] >> 24) & 0x7;

        for (size_t i = 0; i < 8; i++) TALEA_LOG_TRACE("arg%d: %016llx\n", i, cmd->args[i]);
        TALEA_LOG_TRACE("arg1: %016llx, x: %d, y: %d\n", cmd->args[1], x, y);

        TALEA_LOG_TRACE("Blitting\n");
        Video_Blit(m, v, dest, src, w, h, x, y, w, h, rotation);

        break;
    }
    case COMMAND_STRETCH_BLIT: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0:
        //      3 bits for the rotation mode
        //      5 bits for the destination context pointer
        // TODO: document that blits can only be stretched on the first 32 surfaces
        //  Takes 1 sesqui GPU1-3 for the buffer address
        //  Takes 1 half word on GPU4-5 for the buffer width
        //  Takes 1 half word on GPU6-7 for the buffer height
        // Arg 1:
        //  Takes 1 half word on GPU0-1 for the dest x
        //  Takes 1 half word on GPU2-3 for the dest y
        //  Takes 1 half word on GPU4-5 for the dest w
        //  Takes 1 half word on GPU6-7 for the dest h

        if (cmd->argc > 2) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 2) {
            goto not_enough_args;
            break;
        }

        u8 gpu0 = cmd->args[0] >> 56;
        u8 ctx  = gpu0 & 0x1f;

        enum VideoSpriteRotation rotation = gpu0 >> 5;

        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u32 src_addr = (cmd->args[0] >> 32) & 0x00ffffff;
        u8 *src      = &m->main_memory[src_addr];

        u16 w = cmd->args[0] >> 16;
        u16 h = cmd->args[0];

        u16 x      = cmd->args[1] >> 48;
        u16 y      = cmd->args[1] >> 32;
        u16 dest_w = cmd->args[1] >> 16;
        u16 dest_h = cmd->args[1];

        Video_Blit(m, v, dest, src, w, h, x, y, dest_w, dest_h, rotation);

        break;
    }

    case COMMAND_PATTERN_FILL: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0:
        //      3 bits for the rotation mode
        //      5 bits for the destination context pointer
        // TODO: document that pattern fills can only be done on the first 32 surfaces
        //  Takes 1 sesqui GPU1-3 for the pattern address
        //  Takes 1 byte on GPU4 for the pattern width (max pattern is 256x256)
        //  Takes 1 byte on GPU5 for the pattern height
        //  Takes 1 byte on GPU6 for the u offset (scroll x)
        //  Takes 1 byte on GPU7 for the v offset (scroll y)
        // Arg 1:
        //  Takes 1 half word on GPU0-1 for the dest x
        //  Takes 1 half word on GPU2-3 for the dest y
        //  Takes 1 half word on GPU4-5 for the dest w
        //  Takes 1 half word on GPU6-7 for the dest h

        if (cmd->argc > 2) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 2) {
            goto not_enough_args;
            break;
        }

        u8 gpu0 = cmd->args[0] >> 56;
        u8 ctx  = gpu0 & 0x1f;

        enum VideoSpriteRotation rotation = gpu0 >> 5;

        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u32 src_addr = (cmd->args[0] >> 32) & 0x00ffffff;
        u8 *src      = &m->main_memory[src_addr];

        u8 w     = cmd->args[0] >> 24;
        u8 h     = cmd->args[0] >> 16;
        u8 u_off = cmd->args[0] >> 8;
        u8 v_off = cmd->args[0];

        u16 x      = cmd->args[1] >> 48;
        u16 y      = cmd->args[1] >> 32;
        u16 dest_w = cmd->args[1] >> 16;
        u16 dest_h = cmd->args[1];

        Video_PatternFill(m, v, dest, src, w, h, u_off, v_off, x, y, dest_w, dest_h, rotation);

        break;
    }

    // GPU commands (queued)
    case COMMAND_DRAW_RECT: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 byte GPU for the color of the rect
        //  Takes 1 half word on GPU4-5 for the rect width
        //  Takes 1 half word on GPU6-7 for the rect height
        // Arg 1:
        //  Takes 1 half word on GPU0-1 for the dest x
        //  Takes 1 half word on GPU2-3 for the dest y

        u8             ctx  = cmd->args[0] >> 56;
        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u8 color = (cmd->args[0] >> 48) & 0xff;

        u16 h = cmd->args[0] >> 16;
        u16 w = cmd->args[0];

        u16 x = cmd->args[1] >> 48;
        u16 y = cmd->args[1] >> 32;

        TALEA_LOG_TRACE("Drawing Rect color: %d, h: %d, w: %d, x: %d, y: %d\n", color, h, w, x, y);
        Video_DrawRect(m, v, dest, color, x, y, w, h);

        break;
    }
    case COMMAND_DRAW_LINE: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 byte GPU for the color of the line
        //  Takes 1 half word on GPU4-5 for the line x0
        //  Takes 1 half word on GPU6-7 for the line y0
        // Arg 1:
        //  Takes 1 half word on GPU0-1 for the line x1
        //  Takes 1 half word on GPU2-3 for the line y1

        u8             ctx  = cmd->args[0] >> 56;
        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u8 color = (cmd->args[0] >> 48) & 0xff;

        u16 x0 = cmd->args[0] >> 16;
        u16 y0 = cmd->args[0];

        u16 x1 = cmd->args[1] >> 48;
        u16 y1 = cmd->args[1] >> 32;

        TALEA_LOG_TRACE("Drawing line color: %d, x0: %d, y0: %d, x1: %d, y1: %d\n", color, x0, y0,
                        x1, y1);
        Video_DrawLine(m, v, dest, color, x0, y0, x1, y1);

        break;
    }
    case COMMAND_DRAW_CIRCLE: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 byte in GPU1 for the color of the circle line
        //  Takes 1 byte in GPU2 for the color of the circle interior
        //  Takes 1 byte in GPU3 for whether to draw the outline, to fill, or both
        //  Takes 1 half word on GPU4-5 for the circle xm
        //  Takes 1 half word on GPU6-7 for the circle ym
        // Arg 1:
        //  Takes 1 half word on GPU0-1 for the circle radius

        u8             ctx  = cmd->args[0] >> 56;
        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u8 color_line = (cmd->args[0] >> 48) & 0xff;
        u8 color_fill = (cmd->args[0] >> 40) & 0xff;
        u8 mode       = (cmd->args[0] >> 32) & 0xff;

        u16 xm = cmd->args[0] >> 16;
        u16 ym = cmd->args[0];

        u16 r = cmd->args[1] >> 48;

        TALEA_LOG_TRACE("Drawing circle color_line: %d, color_fill: %d, xm: %d, ym: %d, r: %d\n",
                        color_line, color_fill, xm, ym, r);
        Video_DrawCircle(m, v, dest, mode, color_line, color_fill, xm, ym, r);

        break;
    }
    case COMMAND_DRAW_TRI: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 byte GPU1 for the color of the triangle
        //  Takes 1 half word on GPU4-5 for the triangle x0
        //  Takes 1 half word on GPU6-7 for the triangle y0
        // Arg 1:
        //  Takes 1 half word on GPU0-1 for the dest x1
        //  Takes 1 half word on GPU2-3 for the dest y1
        //  Takes 1 half word on GPU4-5 for the dest x2
        //  Takes 1 half word on GPU6-7 for the dest y2

        u8             ctx  = cmd->args[0] >> 56;
        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u8 color = (cmd->args[0] >> 48) & 0xff;

        u16 x0 = cmd->args[0] >> 16;
        u16 y0 = cmd->args[0];

        u16 x1 = cmd->args[1] >> 48;
        u16 y1 = cmd->args[1] >> 32;
        u16 x2 = cmd->args[1] >> 16;
        u16 y2 = cmd->args[1];

        // TALEA_LOG_TRACE("Drawing Tri color: %d, x0: %d, y0: %d, x1: %d, y1: %d, x2: %d, y2:
        // %d\n",
        //                 color, x0, y0, x1, y1, x2, y2);
        Video_DrawTri(m, v, dest, color, x0, y0, x1, y1, x2, y2);

        break;
    }
    case COMMAND_DRAW_BATCH: break;
    case COMMAND_FILL_SPAN: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 byte GPU1 for the color of the line
        //  Takes 1 half word on GPU2-3 for the line x0
        //  Takes 1 half word on GPU4-5 for the line x1
        //  Takes 1 half word on GPU6-7 for the line y

        u8             ctx  = cmd->args[0] >> 56;
        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u8 color = (cmd->args[0] >> 48) & 0xff;

        u16 x0 = cmd->args[0] >> 32;
        u16 x1 = cmd->args[0] >> 16;
        u16 y  = cmd->args[0];

        Video_DrawHorizontalLine(m, v, dest, color, x0, x1, y);
        break;
    }
    case COMMAND_FILL_VSPAN: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 byte GPU1 for the color of the line
        //  Takes 1 half word on GPU2-3 for the line x
        //  Takes 1 half word on GPU4-5 for the line y0
        //  Takes 1 half word on GPU6-7 for the line y1

        u8             ctx  = cmd->args[0] >> 56;
        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        u8 color = (cmd->args[0] >> 48) & 0xff;

        u16 x  = cmd->args[0] >> 32;
        u16 y0 = cmd->args[0] >> 16;
        u16 y1 = cmd->args[0];

        Video_DrawVerticalLine(m, v, dest, color, x, y0, y1);
        break;
    }
    case COMMAND_SET_CSR: {
        // TODO: Document
        // Takes 1 byte on GPU0 for the csr.
        u8 value = cmd->args[0] >> 56;

        v->vblank_enable = value & VIDEO_VBLANK_EN;
        v->cursor_enable = (value & VIDEO_CURSOR_EN) >> 2;
        v->cursor_blink  = (value & VIDEO_CURSOR_BLINK) >> 3;
        v->rop           = (value & (VIDEO_ROP2 | VIDEO_ROP1 | VIDEO_ROP0)) >> 4;

        v->csr = ASSEMBLE_CSR(v);

        break;
    }
    default: TALEA_LOG_ERROR("Unknown video command, cmd: %d\n", cmd->op); break;
    }
    v->last_executed_command = cmd->op;
    return;

too_many_args:
    TALEA_LOG_WARNING("Video, too many args: %d\n", cmd->argc);
    v->error = VIDEO_TOO_MANY_ARGS;
    return;
not_enough_args:
    TALEA_LOG_WARNING("Video, not enough args: %d\n", cmd->argc);
    v->error = VIDEO_NOT_ENOUGH_ARGS;
    return;
}

static void Video_ExecuteCommands(TaleaMachine *m)
{
    DeviceVideo     *v = &m->video;
    struct VideoCmd *cmd;

    while (cmd = Video_PopCommandQueue(m)) {
        Video_ExecuteCommand(m, v, cmd);
    }
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

    TALEA_LOG_TRACE("Pushing video command: %d, argc: %d\n", op, cmd->argc);

    v->cmd_queue.cmd[v->cmd_queue.head].op   = op;
    v->cmd_queue.cmd[v->cmd_queue.head].argc = cmd->argc;
    memcpy(v->cmd_queue.cmd[v->cmd_queue.head].args, cmd->args,
           VIDEO_CMD_MAX_ARGS * sizeof(cmd->args));

    v->cmd_queue.head = next_head;
}

static enum VideoError Video_ProcessCommand(TaleaMachine *m, u8 value)
{
    // TODO: document
    DeviceVideo *v = &m->video;

    switch (value) {
    case COMMAND_END_DRAWING: v->is_drawing = false; break;
    case COMMAND_BEGIN_DRAWING:
        TALEA_LOG_TRACE("begin draw\n");
        v->is_drawing = true;
        break;

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

        v->last_executed_command = value;
        break;

    case COMMAND_BIND_CTX: {
        // TODO: document
        // Binds a 2D buffer (address, w, h) to a context slot
        // Takes 1 byte of context slot in GPU0
        // Takes 1 sesqui of the buffer address in GPU1-3
        // Takes 1 halfword in GPU4-5 for the buffer w
        // Takes 1 halfword in GPU6-7 for the buffer h
        // Cannot bind slot 0!

        u8 slot = v->current_cmd.args[0] >> 56;

        if (slot == 0) return;

        u32 addr = (v->current_cmd.args[0] >> 32) & 0x00ffffff;

        u16 w = v->current_cmd.args[0] >> 16;
        u16 h = v->current_cmd.args[0];

        TALEA_LOG_TRACE("Binding to slot: %d, addr: %04x, w: %d, h: %d\n", slot, addr, w, h);
        v->ctx[slot].addr   = addr;
        v->ctx[slot].w      = w;
        v->ctx[slot].h      = h;
        v->ctx[slot].stride = 1;
        v->ctx[slot].size   = w * h;

        break;
    }

    // check if the target buffer is not the framebuffer
    case COMMAND_BLIT:
    case COMMAND_STRETCH_BLIT:
    case COMMAND_PATTERN_FILL:
    case COMMAND_DRAW_RECT:
    case COMMAND_DRAW_LINE:
    case COMMAND_DRAW_CIRCLE:
    case COMMAND_DRAW_TRI:
    case COMMAND_DRAW_BATCH:
    case COMMAND_FILL_SPAN: {
        // This is very ugly
        u8 ctx = v->current_cmd.args[0] >> 56;
        if (value == COMMAND_STRETCH_BLIT || value == COMMAND_PATTERN_FILL) ctx &= 0x1f;

        if (ctx != 0) {
            v->current_cmd.op = value;
            Video_ExecuteCommand(m, v, &v->current_cmd);
        } else {
            Video_PushCommandQueue(m, value, &v->current_cmd);
        }
        break;
    }

    default: Video_PushCommandQueue(m, value, &v->current_cmd);
    }

    // TODO: doocument that writing ANY command, even not queued resets the argument list
    v->current_cmd.argc = 0;
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
        v->cursor_enable = (value & VIDEO_CURSOR_EN) >> 2;
        v->cursor_blink  = (value & VIDEO_CURSOR_BLINK) >> 3;
        v->rop           = (value & (VIDEO_ROP2 | VIDEO_ROP1 | VIDEO_ROP0)) >> 4;

        v->csr = ASSEMBLE_CSR(v);

        if (value & VIDEO_RESET_REGS) {
            memset(&m->data_memory[P_VIDEO_GPU0], 0, 11);
        }

        break;
    }
    case P_VIDEO_GPU7: {
        if (v->is_drawing) {
            // FIXME: Why is this not filling arg with the values, and instead with zeroes?
            u64 arg;
            arg = ((u64)m->data_memory[P_VIDEO_GPU0]) << 56;
            arg |= ((u64)m->data_memory[P_VIDEO_GPU1]) << 48;
            arg |= ((u64)m->data_memory[P_VIDEO_GPU2]) << 40;
            arg |= ((u64)m->data_memory[P_VIDEO_GPU3]) << 32;
            arg |= ((u64)m->data_memory[P_VIDEO_GPU4]) << 24;
            arg |= ((u64)m->data_memory[P_VIDEO_GPU5]) << 16;
            arg |= ((u64)m->data_memory[P_VIDEO_GPU6]) << 8;
            arg |= value;
            v->current_cmd.args[v->current_cmd.argc] = arg;
            // TALEA_LOG_TRACE("Queueing arg%d: (%016llx) %016llx\n", v->current_cmd.argc, arg,
            //                 v->current_cmd.args[v->current_cmd.argc]);
            memset(&m->data_memory[P_VIDEO_GPU0], 0, 8);
            v->current_cmd.argc++;
        }

        break;
    }
    case P_VIDEO_ERROR: break;
    default: m->data_memory[addr] = value;
    }
}

static u8 pixels[TALEA_SCREEN_HEIGHT * TALEA_SCREEN_WIDTH] = { 0 };

void Video_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    if (!is_restart) m->video = (DeviceVideo){ 0 };

    struct FrontendState *state = Frontend_GetState();

    m->video = (DeviceVideo){
        .mode                  = VIDEO_MODE_TEXT_AND_GRAPHIC,
        .error                 = VIDEO_NO_ERROR,
        .rop                   = VIDEO_CONFIG_ROP_COPY,
        .last_executed_command = COMMAND_NOP,

        .transparency  = 0,
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
                .framebuffer = LoadRenderTexture(TALEA_SCREEN_WIDTH, TALEA_SCREEN_HEIGHT),
                .pixels =
                    LoadTextureFromImage((Image){ .data    = pixels,
                                                  .width   = TALEA_SCREEN_WIDTH,
                                                  .height  = TALEA_SCREEN_HEIGHT,
                                                  .mipmaps = 1,
                                                  .format  = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE }),
                .background_color = BLACK,
                .foreground_color = GOLD,
                .screen_texture   = &state->window.screenTexture,
            },
    };

    memcpy(&m->video.renderer.shaderPalette, Palette_Default_Aurora, sizeof(u32) * 4 * (256));

    Video_PrepareFont(&m->video, VIDEO_FONT_BASE_CP0,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/cp0.fnt"));
    Video_PrepareFont(&m->video, VIDEO_FONT_BASE_CP1,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/cp1.fnt"));
    Video_PrepareFont(&m->video, VIDEO_ALT_FONT_CP0,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/alt_cp0.fnt"));
    Video_PrepareFont(&m->video, VIDEO_ALT_FONT_CP1,
                      TextFormat("%s%s%s", FONT_PATH, config->hardware_font, "/alt_cp1.fnt"));

    Video_Clear(m, &m->video, 0x20000004, 0);
    SetTextureFilter(m->video.renderer.pixels, TEXTURE_FILTER_POINT);
    Video_RendererInit(&m->video, SHADERS_PATH("mode_text_color.fs"));
}
