#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "core/bus.h"
#include "frontend/frontend.h"
#include "palettes.h"
#include "talea.h"
#include "video.h"

#define ASSEMBLE_CSR(v)                                                  \
    (((v)->queueFull << 7) | ((v)->rop << 4) | ((v)->cursorBlink << 3) | \
     ((v)->cursorEnable << 2) | 0 | (v)->vblankEnable)

static inline getBitXY(const u8 *bitmap, u8 charIndex, u8 x, u8 y, u8 charW, u8 charH)
{
    size_t bytesPerRow = (charW + 7) / 8;
    size_t glyphStart  = charIndex * (bytesPerRow * charH);
    size_t rowStart    = glyphStart + (y * bytesPerRow);
    return (bitmap[rowStart + (x / 8)] >> (7 - (x % 8))) & 1;
}

static Image Video_Load1BitFont(const u8 *data, u8 charW, u8 charH, size_t nchars, u8 startingGlyph,
                                u8 defaultGlyph)
{
    // data is 1 BIT per pixel, aligned by row: 0 is blank 1 is filled

    size_t atlasW = (u32)charW, atlasH = (u32)charH * 256; // always 256 chars
    size_t bytesPerRow   = ((u32)charW + 7) / 8;
    size_t bytesPerGlyph = bytesPerRow * charH;
    Image  atlas         = GenImageColor(atlasW, atlasH, BLANK);
    Color *pixels        = atlas.data;

    u8 *defaultBits = malloc(bytesPerGlyph);
    if (defaultBits == NULL) {
        TALEA_LOG_ERROR("Malloc failed allocating defaultBits!\n");
        return (Image){ 0 };
    }

    memcpy(defaultBits, &data[(defaultGlyph * bytesPerGlyph)], bytesPerGlyph);

    // populate the atlas
    for (size_t row = 0; row < atlasH; row++) {
        size_t currentGlyph = row / charH;
        for (size_t col = 0; col < atlasW; col++) {
            u8 bit = 0;
            if (currentGlyph < startingGlyph || currentGlyph >= nchars)
                bit = getBitXY(defaultBits, 0, col, row % charH, charW, charH);
            else
                bit = getBitXY(data, currentGlyph - startingGlyph, col, row % charH, charW, charH);
            pixels[(row * atlasW) + col] = bit ? WHITE : BLANK;
        }
    }

    // we pass firstChar as 0 because we have already accounted for that
    bool success = ExportImage(atlas, "testfont.png");
    if (!success) TALEA_LOG_TRACE("COULD NOT SAVE THE PIC\n");
    free(defaultBits);
    return atlas;
}

u8 Video_Read(TaleaMachine *m, u8 port)
{
    DeviceVideo *v = &m->video;

    switch (port & 0xf) {
    case P_VIDEO_COMMAND: return v->lastExecutedCommand;
    // TODO: Ensure we do indexes everywhere
    case P_VIDEO_CUR_X: return v->cursorCellIndex % v->textbuffer.w;
    case P_VIDEO_CUR_Y: return v->cursorCellIndex / v->textbuffer.w;
    case P_VIDEO_CSR: {
        v->csr = ASSEMBLE_CSR(v);
        return v->csr;
    }
    case P_VIDEO_ERROR: return v->error;
    default: return v->ports[port & 0xf];
    }
}

// We support only PSF 1 and 2 currently

// taken from: https://wiki.osdev.org/PC_Screen_Font
#define PSF1_FONT_MAGIC 0x0436

typedef struct {
    u16 magic;         // Magic bytes for identification.
    u8  fontMode;      // PSF font mode.
    u8  characterSize; // PSF character size.
} PSF1Header;

#define PSF2_FONT_MAGIC 0x864ab572

/*
    PSF2
*/
typedef struct {
    u32 magic;         /* magic bytes to identify PSF */
    u32 version;       /* zero */
    u32 headersize;    /* offset of bitmaps in file, 32 */
    u32 flags;         /* 0 if there's no unicode table */
    u32 numglyph;      /* number of glyphs */
    u32 bytesperglyph; /* size of each glyph */
    u32 height;        /* height in pixels */
    u32 width;         /* width in pixels */
} PSF2_Header;

bool Video_PrepareFont(TaleaMachine *m, DeviceVideo *v, enum VideoFontID id, const char *path)
{
    u8     charW, charH, startingGlyph, defaultGlyph;
    size_t nchars; // MAX 256!
    u8    *glyphData  = NULL;
    int    textureCol = 0;

    int sz;
    u8 *fontFile = LoadFileData(path, &sz);

    if (((u32 *)fontFile)[0] == PSF2_FONT_MAGIC) {
        nchars        = ((u32 *)fontFile)[4];
        charH         = ((u32 *)fontFile)[6];
        charW         = ((u32 *)fontFile)[7];
        startingGlyph = 0;
        defaultGlyph  = '?';
        glyphData     = fontFile + ((u32 *)fontFile)[2];

        size_t bytesPerRow   = ((u32)charW + 7) / 8;
        size_t bytesPerGlyph = bytesPerRow * charH;
        if (bytesPerGlyph != ((u32 *)fontFile)[5]) {
            TALEA_LOG_ERROR("Font %s indicates unexpected bytes per glyph\n", path);
            return false;
        }
    } else if (((u16 *)fontFile)[0] == PSF1_FONT_MAGIC) {
        charW         = 8;
        charH         = fontFile[3];
        startingGlyph = 0;
        defaultGlyph  = '?';
        glyphData     = fontFile + 4;
        nchars        = (fontFile[2] & 1) ? 512 : 256;
    } else {
        TALEA_LOG_ERROR("We only support PSF1 and a subset of PSF2 (%s)\n", path);
        return false;
    }

    if (nchars > 256) {
        TALEA_LOG_WARNING(
            "Cannot load font with more than 256 glyphs. only first 256 glyphs loaded!(%s)\n",
            path);
        nchars = 256;
    }

    if (charW < 4 || charH < 4) {
        TALEA_LOG_ERROR("This font (%s) is very little! Make it bigger than 4x4\n", path);
        return false;
    }

    // CHECK AGAINST BASE_CP0 (if not itself)
    // Reject or proceed update master texture
    if (id != VIDEO_FONT_BASE_CP0) {
        if (charW != v->font.charW || charH != v->font.charH) {
            TALEA_LOG_ERROR(
                "Fonts have to have all the same witdht and height. Try to load this one (%s) as BASE CP0!\n",
                path);
            return false;
        }

        switch (id) {
        case VIDEO_FONT_BASE_CP1: textureCol = charW * 1; break;
        case VIDEO_ALT_FONT_CP0: textureCol = charW * 2; break;
        case VIDEO_ALT_FONT_CP1: textureCol = charW * 3; break;
        default: TALEA_LOG_TRACE("Malformed font id (should be 1..3) %d\n", id); return false;
        }

    } else {
        // Do this if and only if we're loading the primary font
        // Calculate character sizes. MUST be equal for all 4 fonts

        // wipe texture.
        UnloadRenderTexture(v->font.atlas);

        v->textbuffer.w      = v->framebuffer.w / charW;
        v->textbuffer.h      = v->framebuffer.h / charH;
        v->textbuffer.stride = v->mode == VIDEO_MODE_TEXT_MONO ? 1 : 4;
        v->textbuffer.view   = Bus_GetView(m, v->textbuffer.view.guest_addr,
                                           v->textbuffer.h * v->textbuffer.w * v->textbuffer.stride,
                                           BUS_ACCESS_READ | BUS_ACCESS_WRITE);

        if (!v->textbuffer.view.ptr) {
            TALEA_LOG_WARNING("Prepare Font, unable to get textbuffer view\n");
            return false;
        }

        v->font.charW = charW;
        v->font.charH = charH;
        v->font.atlas = LoadRenderTexture(charW * VIDEO_FONT_IDS, charH * 256);
        SetTextureFilter(v->font.atlas.texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(v->font.atlas.texture, TEXTURE_WRAP_CLAMP);

        UnloadRenderTexture(v->renderer.charactersTexture);
        v->renderer.charactersTexture = LoadRenderTexture(v->textbuffer.w, v->textbuffer.h);
        SetTextureFilter(v->renderer.charactersTexture.texture, TEXTURE_FILTER_POINT);

        free(v->renderer.charsFake);
        v->renderer.charsFake = malloc((size_t)v->textbuffer.w * v->textbuffer.h * sizeof(Color));

        textureCol = 0;
    }

    Image subAtlas =
        Video_Load1BitFont(glyphData, charW, charH, nchars, startingGlyph, defaultGlyph);

    if (!subAtlas.data || subAtlas.height == 0 || subAtlas.width == 0) {
        TALEA_LOG_ERROR("Failed to load font as cp0: %s\n", path);
        return false;
    }

    // update the master texture
    BeginTextureMode(v->font.atlas);
    Texture2D subAtlasTexture = LoadTextureFromImage(subAtlas);
    DrawTexture(subAtlasTexture, textureCol, 0, WHITE);
    EndTextureMode();
    UnloadTexture(subAtlasTexture);
    UnloadImage(subAtlas);

    TALEA_LOG_TRACE("Loaded font '%s' w: %d, h:%d, tb:%dx%d chars \n", path, charW, charH,
                    v->textbuffer.w, v->textbuffer.h);
    return true;
}

bool Video_PrepareFontFromMemory(TaleaMachine *m, DeviceVideo *v, enum VideoFontID id, u8 *data,
                                 u8 charW, u8 charH, size_t nchars, u8 startingGlyph,
                                 u8 defaultGlyph)
{
    int textureCol = 0;

    if (nchars > 256) {
        TALEA_LOG_WARNING(
            "Cannot load font from memory with more than 256 glyphs. only first 256 glyphs loaded!(%d)\n",
            id);
        nchars = 256;
    }

    if (charW < 4 || charH < 4) {
        TALEA_LOG_ERROR("This memory font (%d) is very little! Make it bigger than 4x4\n", id);
        return false;
    }

    // CHECK AGAINST BASE_CP0 (if not itself)
    // Reject or proceed update master texture
    if (id != VIDEO_FONT_BASE_CP0) {
        if (charW != v->font.charW || charH != v->font.charH) {
            TALEA_LOG_ERROR(
                "Fonts have to have all the same witdht and height. Try to load this one (%d) as BASE CP0!\n",
                id);
            return false;
        }

        switch (id) {
        case VIDEO_FONT_BASE_CP1: textureCol = charW * 1; break;
        case VIDEO_ALT_FONT_CP0: textureCol = charW * 2; break;
        case VIDEO_ALT_FONT_CP1: textureCol = charW * 3; break;
        default: TALEA_LOG_TRACE("Malformed font id (should be 1..3) %d\n", id); return false;
        }

    } else {
        // Do this if and only if we're loading the primary font
        // Calculate character sizes. MUST be equal for all 4 fonts

        // wipe texture.
        UnloadRenderTexture(v->font.atlas);

        v->textbuffer.w      = v->framebuffer.w / charW;
        v->textbuffer.h      = v->framebuffer.h / charH;
        v->textbuffer.stride = v->mode == VIDEO_MODE_TEXT_MONO ? 1 : 4;
        v->textbuffer.view   = Bus_GetView(m, v->textbuffer.view.guest_addr,
                                           v->textbuffer.h * v->textbuffer.w * v->textbuffer.stride,
                                           BUS_ACCESS_READ | BUS_ACCESS_WRITE);

        if (!v->textbuffer.view.ptr) {
            TALEA_LOG_WARNING("Prepare Font dorm Memory, unable to get textbuffer view\n");
            return false;
        }

        v->font.charW = charW;
        v->font.charH = charH;
        v->font.atlas = LoadRenderTexture(charW * VIDEO_FONT_IDS, charH * 256);
        SetTextureFilter(v->font.atlas.texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(v->font.atlas.texture, TEXTURE_WRAP_CLAMP);

        UnloadRenderTexture(v->renderer.charactersTexture);
        v->renderer.charactersTexture = LoadRenderTexture(v->textbuffer.w, v->textbuffer.h);
        SetTextureFilter(v->renderer.charactersTexture.texture, TEXTURE_FILTER_POINT);

        free(v->renderer.charsFake);
        v->renderer.charsFake = malloc((size_t)v->textbuffer.w * v->textbuffer.h * sizeof(Color));

        textureCol = 0;
    }

    Image subAtlas = Video_Load1BitFont(data, charW, charH, nchars, startingGlyph, defaultGlyph);

    if (!subAtlas.data || subAtlas.height == 0 || subAtlas.width == 0) {
        TALEA_LOG_ERROR("Failed to load memory font as cp0\n");
        return false;
    }

    // update the master texture
    BeginTextureMode(v->font.atlas);
    Texture2D subAtlasTexture = LoadTextureFromImage(subAtlas);
    DrawTexture(subAtlasTexture, textureCol, 0, WHITE);
    EndTextureMode();
    UnloadTexture(subAtlasTexture);
    UnloadImage(subAtlas);

    TALEA_LOG_TRACE("Loaded font from memory: w: %d, h:%d, tb:%dx%d chars \n", charW, charH,
                    v->textbuffer.w, v->textbuffer.h);
    return true;
}

void Video_RendererInit(DeviceVideo *v, const char *path)
{
    // clang-format off

    v->renderer.shader          = LoadShader(VERTEX_SHADER, path);
    v->renderer.fbShader          = LoadShader(VERTEX_SHADER, SHADERS_PATH("fb_pixels.fs"));
    
    v->renderer.cursorCellIdxLoc = GetShaderLocation(v->renderer.shader, "cursorIndex");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "cursorIndex", v->renderer.cursorCellIdxLoc);
    v->renderer.csrLoc   = GetShaderLocation(v->renderer.shader, "csr");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "csr", v->renderer.csrLoc);
    v->renderer.charsLoc       = GetShaderLocation(v->renderer.shader, "characters");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "characters", v->renderer.charsLoc);
    v->renderer.fontsLoc = GetShaderLocation(v->renderer.shader, "font_atlas");
    v->renderer.paletteLoc     = GetShaderLocation(v->renderer.shader, "palette");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "palette", v->renderer.paletteLoc);
    v->renderer.timeLoc        = GetShaderLocation(v->renderer.shader, "uTime");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "uTime", v->renderer.timeLoc);
    v->renderer.charSizeLoc    = GetShaderLocation(v->renderer.shader, "charSize");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "charSize", v->renderer.charSizeLoc);
    v->renderer.textureSizeLoc = GetShaderLocation(v->renderer.shader, "mainTextureSize");
    TALEA_LOG_ERROR("Shader location for %s: %d\n",  "mainTextureSize", v->renderer.textureSizeLoc);
    TALEA_LOG_ERROR("Shader location for texture0: %d\n",  GetShaderLocation(v->renderer.shader, "texture0"));

    SetShaderValue(v->renderer.shader, v->renderer.cursorCellIdxLoc, &v->cursorCellIndex, SHADER_UNIFORM_INT);
    SetShaderValue(v->renderer.shader, v->renderer.csrLoc, &v->csr, SHADER_UNIFORM_INT);
    SetShaderValueV(v->renderer.shader, v->renderer.paletteLoc, v->renderer.shaderPalette, SHADER_UNIFORM_IVEC4, 256);

    v->renderer.fbLoc = GetShaderLocation(v->renderer.fbShader, "pixels");
    v->renderer.fbPaletteLoc = GetShaderLocation(v->renderer.fbShader, "palette");
    SetShaderValueV(v->renderer.fbShader, v->renderer.fbPaletteLoc, v->renderer.shaderPalette, SHADER_UNIFORM_IVEC4, 256);

    // clang-format on
}

#define SHADE_LEVELS 16
static void Video_BuildShadesTable(u8 *dest, u32 *indexed_colors, size_t color_count)
{
    // We assume we're using the format of an u32 PER byte in the indexed colors pallete
    // And we also hardcode 16 levels of shading

    for (size_t i = 0; i < color_count; i++) {
        u32 r = indexed_colors[(i * 4)];
        u32 g = indexed_colors[(i * 4) + 1];
        u32 b = indexed_colors[(i * 4) + 2];

        for (size_t level = 0; level < SHADE_LEVELS; level++) {
            float brightness = (float)level / (float)(SHADE_LEVELS - 1);

            float target_r = r * brightness;
            float target_g = g * brightness;
            float target_b = b * brightness;

            size_t best_match   = 0;
            float  min_distance = INFINITY;

            for (size_t k = 0; k < color_count; k++) {
                u32 pr = indexed_colors[(k * 4)];
                u32 pg = indexed_colors[(k * 4) + 1];
                u32 pb = indexed_colors[(k * 4) + 2];

                float dr = target_r - pr;
                float dg = target_g - pg;
                float db = target_b - pb;

                float dist_sq = (dr * dr * 2) + (dg * dg * 4) + (db * db * 3);

                if (dist_sq < min_distance) {
                    min_distance = dist_sq;
                    best_match   = k;
                }
            }

            dest[(i * SHADE_LEVELS) + level] = (u8)best_match;
        }
    }
}

static inline void TextMode_AssembleCharacters(TaleaMachine *m)
{
    DeviceVideo *v = &m->video;

    if (v->mode == VIDEO_MODE_TEXT_COLOR || v->mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        /*for (size_t i = 0, k = 0; i < v->textbuffer.w * v->textbuffer.h; i++, k += 4) {
            v->renderer.charsFake[i] = (Color){
                v->textbuffer.view.ptr[k],
                v->textbuffer.view.ptr[k + 1],
                v->textbuffer.view.ptr[k + 2],
                v->textbuffer.view.ptr[k + 3],
            };
        }*/
        UpdateTexture(v->renderer.charactersTexture.texture, v->textbuffer.view.ptr);
    } else if (v->mode == VIDEO_MODE_TEXT_MONO) {
        for (size_t i = 0; i < v->textbuffer.w * v->textbuffer.h; i++) {
            v->renderer.charsFake[i] = (Color){
                v->textbuffer.view.ptr[i],
                0,
                0,
                0,
            };
        }
    }
}

static inline void Framebuffer_Update(TaleaMachine *m)
{
    DeviceVideo   *v = &m->video;
    VideoRenderer *r = &m->video.renderer;

    UpdateTexture(r->pixels, v->framebuffer.view.ptr);
    BeginShaderMode(r->fbShader);
    SetShaderValueTexture(r->fbShader, r->fbLoc, r->pixels);
    DrawTexture(r->pixels, 0, 0, WHITE);
    EndShaderMode();

    DrawTexturePro(r->framebuffer.texture,
                   (Rectangle){ 0.0, 0.0f, (float)r->framebuffer.texture.width,
                                (float)r->framebuffer.texture.height },
                   (Rectangle){ 0.0, 0.0f, (float)r->screenTexture->texture.width,
                                (float)r->screenTexture->texture.height },
                   (Vector2){ 0, 0 }, 0.0f, WHITE);
}

static inline void TextMode_Render(TaleaMachine *m)
{
    DeviceVideo *v = &m->video;

    if (v->mode == VIDEO_MODE_TEXT_MONO /* || v->mode == VIDEO_MODE_TEXT_COLOR ||
        v->mode == VIDEO_MODE_TEXT_AND_GRAPHIC */)
        UpdateTexture(v->renderer.charactersTexture.texture, v->renderer.charsFake);

    float charSize[2]     = { v->font.charW, v->font.charH };
    int   textureSize[2]  = { v->renderer.screenTexture->texture.width,
                              v->renderer.screenTexture->texture.height };
    int   cursorCellIndex = v->cursorCellIndex;
    int   cursorCsr = v->csr = ASSEMBLE_CSR(&m->video);

    BeginShaderMode(v->renderer.shader);

    SetShaderValue(v->renderer.shader, v->renderer.cursorCellIdxLoc, &cursorCellIndex,
                   SHADER_UNIFORM_INT);
    SetShaderValue(v->renderer.shader, v->renderer.csrLoc, &cursorCsr, SHADER_UNIFORM_INT);
    SetShaderValue(v->renderer.shader, v->renderer.baseColorLoc, &v->renderer.foregroundColor,
                   SHADER_UNIFORM_IVEC4);
    SetShaderValue(v->renderer.shader, v->renderer.charSizeLoc, &charSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(v->renderer.shader, v->renderer.textureSizeLoc, &textureSize,
                   SHADER_UNIFORM_IVEC2);
    SetShaderValueTexture(v->renderer.shader, v->renderer.fontsLoc, v->font.atlas.texture);
    SetShaderValueTexture(v->renderer.shader, v->renderer.charsLoc,
                          v->renderer.charactersTexture.texture);

    if (!IsShaderValid(v->renderer.shader))
        TALEA_LOG_ERROR("On UpdateVideo: Shader is not valid\n");

    // Drawing BLANK texture, all magic happens on shader
    DrawTexture(v->renderer.screenTexture->texture, 0, 0, BLANK);
    EndShaderMode(); // Disable our custom shader, return to default shader
}

static struct VideoCmd *Video_PopCommandQueue(TaleaMachine *m)
{
    DeviceVideo     *v = &m->video;
    struct VideoCmd *res;

    if (v->cmdQueue.head != v->cmdQueue.tail) {
        res              = &v->cmdQueue.cmd[v->cmdQueue.tail];
        v->cmdQueue.tail = (v->cmdQueue.tail + 1) % VIDEO_CMD_QUEUE_SIZE;
        return res;
    }

    return NULL;
}

static inline void Video_Clear(TaleaMachine *m, DeviceVideo *v, u32 pattern, u8 color, u8 flags)
{
    // TODO: should acquire a view of the text and framebuffers on initialization AND on every
    // VIDEO_COMMAND_SET_ADDR
    if (v->mode == VIDEO_MODE_TEXT_MONO) {
        Bus_Memset(m, &v->textbuffer.view, ' ', &v->textbuffer.view);
        return;
    }

    if (flags & VIDEO_CLEAR_FLAG_TB) {
        u32 bePattern = toBe32(pattern);
        Bus_Memset32(m, &v->textbuffer.view, bePattern, v->textbuffer.h * v->textbuffer.w);
    }

    if (flags & VIDEO_CLEAR_FLAG_FB) {
        Bus_Memset(m, &v->framebuffer.view, color,
                   (v->framebuffer.h * v->framebuffer.w * v->framebuffer.stride));
    }

    v->cursorCellIndex = 0;
}

static inline void Video_SetMode(TaleaMachine *m, DeviceVideo *v, u8 mode)
{
    // TODO: implement small font
    if (mode > VIDEO_MODE_TEXT_AND_GRAPHIC) {
        v->error = VIDEO_ERROR_NO_MODE;
        return;
    }

    if (mode == VIDEO_MODE_TEXT_MONO) {
        // Shrinking the lenght should not affect the view maybe inplement viewResize
        UnloadShader(v->renderer.shader);
        v->textbuffer.stride      = 1;
        v->textbuffer.view.length = v->textbuffer.stride * v->textbuffer.w * v->textbuffer.h;
        Video_RendererInit(v, SHADERS_PATH("mode_text_mono.fs"));
    } else {
        UnloadShader(v->renderer.shader);
        v->textbuffer.stride = 4;
        v->textbuffer.view =
            Bus_GetView(m, v->textbuffer.view.guest_addr,
                        v->framebuffer.view.length !=
                            v->framebuffer.w * v->framebuffer.h * v->framebuffer.stride,
                        BUS_ACCESS_READ | BUS_ACCESS_WRITE);
        if (!v->framebuffer.view.ptr || v->framebuffer.view.length != v->framebuffer.w *
                                                                          v->framebuffer.h *
                                                                          v->framebuffer.stride) {
            TALEA_LOG_WARNING(
                "Could not acquire memory view at %06x for framebuffer on reset. Consider relocating it\n",
                VIDEO_FRAMEBUFFER_ADDR);
            v->error = VIDEO_ERROR_DMA;
        }
        Video_RendererInit(v, SHADERS_PATH("mode_text_color.fs"));
    }

    v->mode = mode;
    TALEA_LOG_TRACE("Set video mode to mode %d\n", mode);
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
    // TODO: acquire a view of the context on bind!

    u8 *dest_start = dest->view.ptr;
    u8 *dest_end   = dest_start + dest->view.length;

    u8  *temp_src = src;
    bool is_temp  = false;

    if (src >= dest_start && src < dest_end) {
        // buffers overlap
        temp_src = malloc((size_t)w * h * sizeof(u8));
        if (!temp_src) {
            // panic here?
            TALEA_LOG_ERROR("Out of mem blitting\n");
            return;
        }

        memcpy(temp_src, src, (size_t)w * h * sizeof(u8));
        is_temp = true;
    }

    u16 v_x1 = MAX(0, x);
    u16 v_y1 = MAX(0, y);
    u16 v_x2 = MIN((i64)dest->w, x + dest_w);
    u16 v_y2 = MIN((i64)dest->h, y + dest_h);

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
        TALEA_LOG_WARNING("Blit, coordinates outside buffer\n");
        // TODO: maybe raise an error
        return;
    }

    u16 v_x1 = MAX(0, x);
    u16 v_y1 = MAX(0, y);
    u16 v_x2 = MIN(dest->w, x + w);
    u16 v_y2 = MIN(dest->h, y + h);

    if (v_x1 >= v_x2 || v_y1 >= v_y2) return;

    u8 *fb = dest->view.ptr;

    // TALEA_LOG_TRACE("ROP: %d\n", v->rop);
    for (size_t row = v_y1; row < v_y2; row++) {
        u8 *rect = fb + (dest->w * row + v_x1) * dest->stride;
        for (size_t col = v_x1; col < v_x2; col++) {
            Video_ApplyROP(v, &rect[(col - v_x1) * dest->stride], color);
        }
    }
}

static void Video_PatternFill(TaleaMachine *m, DeviceVideo *vid, struct Buff2D *dest, u8 *src, u8 w,
                              u8 h, u8 u_off, u8 v_off, i16 x, i16 y, u16 dest_w, u16 dest_h,
                              enum VideoSpriteRotation rotation)
{
    // TODO: remember to ALWAYS commit before starting these architectural changes...
    u8             *dest_start = dest->view.ptr;
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
    // if (x == 0) TALEA_LOG_TRACE("Setting pixel at x: %d, y: %d!\n", x, y);
    if (x >= dest->w || y >= dest->h) return;
    if (x < 0 || y < 0) return;

    u8    *surface = dest->view.ptr;
    size_t index   = (((size_t)y * dest->w) + x) * dest->stride;
    Video_ApplyROP(v, &surface[index], color);
}

static void inline DrawHorizontalLineNoCheck(DeviceVideo *v, u8 color, u8 *start, i16 len)
{
    // This assumes bounds have been checked, coordinates clipped, and x0 < x1
    if (len <= 0) return;

    if (v->rop == VIDEO_CONFIG_ROP_COPY) {
        memset(start, color, len);
    } else {
        for (size_t col = 0; col < len; col++) {
            Video_ApplyROP(v, &start[col], color);
        }
    }
}

static void Video_DrawHorizontalLine(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                     i16 x0, i16 x1, i16 y)
{
    // TALEA_LOG_TRACE("Drawing scanline %d from %d to %d, color %x\n", y, x1, x2, color);

    if (y < 0 || y >= dest->h) return;

    size_t start_x = MAX(0, MIN(x0, x1));
    size_t end_x   = MIN(dest->w - 1, MAX(x0, x1));

    if (start_x > end_x) return;

    u8 *start = &dest->view.ptr[(y * dest->w) + start_x];
    u16 len   = (end_x - start_x) + 1;

    DrawHorizontalLineNoCheck(v, color, start, len);
}

static void Video_DrawVerticalLine(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                   i16 x, i16 y1, i16 y2)
{
    // TALEA_LOG_TRACE("Drawing vline %d from %d to %d, color %x\n", x, y1, y2, color);

    if (x < 0 || x >= dest->w) return;

    size_t start_y = MAX(0, MIN(y1, y2));
    size_t end_y   = MIN(dest->h - 1, MAX(y1, y2));

    if (start_y > end_y) return;

    u8 *buff = dest->view.ptr;

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
        return;
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
                             u8 colorLine, u8 colorFill, i16 xm, i16 ym, i16 r)
{
    // from http://members.chello.at/easyfilter/bresenham.html

    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    int lastY = -1;
    int lastX = 1;
    int fillR = r - 1;

    do {
        Buff2D_SetPixel(m, v, dest, colorLine, xm - x, ym + y); /*   I. Quadrant */
        Buff2D_SetPixel(m, v, dest, colorLine, xm - y, ym - x); /*  II. Quadrant */
        Buff2D_SetPixel(m, v, dest, colorLine, xm + x, ym - y); /* III. Quadrant */
        Buff2D_SetPixel(m, v, dest, colorLine, xm + y, ym + x); /*  IV. Quadrant */
        r = err;
        if (r <= y) err += ++y * 2 + 1;           /* e_xy+e_y < 0 */
        if (r > x || err > y) err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);

    if (mode & VIDEO_DRAW_MODE_CIRCLE_FILL) {
        Video_DrawFilledCircle(m, v, dest, colorFill, xm, ym, fillR);
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

static void Video_DrawTriBarycentric(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                     i16 x0, i16 y0, i16 x1, i16 y1, i16 x2, i16 y2)
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

static inline void fillBottomFlatTri(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                     bool clip, fx16 x0, fx16 y0, fx16 x1, fx16 y1, fx16 x2,
                                     fx16 y2)
{
    // vertices alreary ordered

    if (y1 <= y0) return; /* BottomFlat: y1 is bottom, y0 is top */

    fx16 dy = y1 - y0;

    fx16 invs0 = (fx16)(((i64)(x1 - x0) << FX16) / dy);
    fx16 invs1 = (fx16)(((i64)(x2 - x0) << FX16) / dy);

    fx16 invl = (invs0 < invs1) ? invs0 : invs1;
    fx16 invr = (invs0 < invs1) ? invs1 : invs0;

    fx16 xl = x0;
    fx16 xr = x0;

    int start_y = FROM_FX16(y0);
    int end_y   = FROM_FX16(y1);

    if (start_y < 0) {
        int prestep = 0 - start_y;
        xl += (fx16)((i64)invl * prestep);
        xr += (fx16)((i64)invr * prestep);
        start_y = 0;
    }

    end_y = end_y > dest->h ? dest->h : end_y;

    u8 *row = &dest->view.ptr[(start_y * dest->w)];

    if (!clip)
        for (size_t scany = start_y; scany < end_y;
             xl += invl, xr += invr, row += dest->w, scany++) {
            DrawHorizontalLineNoCheck(v, color, row + FROM_FX16(xl), FROM_FX16(xr) - FROM_FX16(xl));
        }
    else
        for (size_t scany = start_y; scany < end_y;
             xl += invl, xr += invr, row += dest->w, scany++) {
            int l = FROM_FX16(xl);
            int r = FROM_FX16(xr);

            l = l < 0 ? 0 : l;
            r = r >= dest->w ? dest->w : r;

            if (l < r) DrawHorizontalLineNoCheck(v, color, row + l, r - l);
        }
}

static inline void fillTopFlatTri(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color,
                                  bool clip, fx16 x0, fx16 y0, fx16 x1, fx16 y1, fx16 x2, fx16 y2)
{
    // verices alreary ordered

    if (y2 <= y0) return; /* TopFlat: y2 is bottom, y0 is top */

    fx16 dy1 = y2 - y1;
    fx16 dy0 = y2 - y0;

    fx16 invs0 = (fx16)(((i64)(x2 - x0) << FX16) / dy0);
    fx16 invs1 = (fx16)(((i64)(x2 - x1) << FX16) / dy0);

    fx16 invl, invr, xl, xr;

    xl   = x0;
    xr   = x1;
    invl = invs0;
    invr = invs1;

    if (xr < xl) {
        fx16 t = xl;
        xl     = xr;
        xr     = t;
        t      = invl;
        invl   = invr;
        invr   = t;
    }

    int start_y = FROM_FX16(y0);
    int end_y   = FROM_FX16(y2);

    if (start_y < 0) {
        int prestep = 0 - start_y;
        xl += (fx16)((i64)invl * prestep);
        xr += (fx16)((i64)invr * prestep);
        start_y = 0;
    }

    end_y = end_y >= dest->h ? dest->h - 1 : end_y;

    u8 *row = &dest->view.ptr[((start_y * dest->w))];

    if (!clip)
        for (size_t scany = start_y; scany <= end_y;
             xl += invl, xr += invr, row += dest->w, scany++) {
            DrawHorizontalLineNoCheck(v, color, row + FROM_FX16(xl), FROM_FX16(xr) - FROM_FX16(xl));
        }
    else
        for (size_t scany = start_y; scany <= end_y;
             xl += invl, xr += invr, row += dest->w, scany++) {
            int l = FROM_FX16(xl);
            int r = FROM_FX16(xr);

            l = l < 0 ? 0 : l;
            r = r >= dest->w ? dest->w : r;

            if (l < r) DrawHorizontalLineNoCheck(v, color, row + l, r - l);
        }
}

static void Video_DrawTri(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u8 color, fx16 x0,
                          fx16 y0, fx16 x1, fx16 y1, fx16 x2, fx16 y2)
{
    // adapted from
    // https://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

    // sort vertices
    fx16 tmp_x, tmp_y;
    if (y0 > y1) {
        tmp_x = x0;
        tmp_y = y0;
        x0    = x1;
        y0    = y1;
        x1    = tmp_x;
        y1    = tmp_y;
    }

    if (y0 > y2) {
        tmp_x = x0;
        tmp_y = y0;
        x0    = x2;
        y0    = y2;
        x2    = tmp_x;
        y2    = tmp_y;
    }

    if (y1 > y2) {
        tmp_x = x1;
        tmp_y = y1;
        x1    = x2;
        y1    = y2;
        x2    = tmp_x;
        y2    = tmp_y;
    }

    if (FROM_FX16(y2) < 0 || FROM_FX16(y0) >= dest->h) return;
    if (y0 == y2) return;

    int  minx = FROM_FX16(MIN(x0, MIN(x1, x2)));
    int  maxx = FROM_FX16(MAX(x0, MAX(x1, x2)));
    bool clip = (minx < 0) || (maxx >= dest->w - 1);

    // sorted, y1 <= y2 <= 3
    /* check for trivial case of bottom-flat triangle */
    if (y1 == y2) {
        fillBottomFlatTri(m, v, dest, color, clip, x0, y0, x1, y1, x2, y2);
    }
    /* check for trivial case of top-flat triangle */
    else if (y0 == y1) {
        fillTopFlatTri(m, v, dest, color, clip, x0, y0, x1, y1, x2, y2);
    } else {
        /* general case - split the triangle in a topflat and bottom-flat one */
        i64 dx         = (i64)x2 - x0;
        i64 dy_total   = (i64)y2 - y0;
        i64 dy_partial = (i64)y1 - y0;

        if (dy_total <= 0) return;

        fx16 x3 = x0 + (fx16)((dy_partial * dx) / dy_total);
        fx16 y3 = y1;

        fillBottomFlatTri(m, v, dest, color, clip, x0, y0, x1, y1, x3, y3);
        fillTopFlatTri(m, v, dest, color, clip, x1, y1, x3, y3, x2, y2);
    }
}

static inline bool CohenSutherland(struct Buff2D *dest, i16v2 *p0, i16v2 *p1)
{
    enum { Inside = 0, Left = 1 << 0, Right = 1 << 1, Bottom = 1 << 2, Top = 1 << 3 };

    i16 minx = 0, maxx = dest->w - 1;
    i16 miny = 0, maxy = dest->h - 1;

    while (true) {
        u8 p0_regions = Inside;
        p0_regions |= p0->x < minx ? Left : p0->x > maxx ? Right : 0;
        p0_regions |= p0->y < miny ? Bottom : p0->y > maxy ? Top : 0;

        u8 p1_regions = Inside;
        p1_regions |= p1->x < minx ? Left : p1->x > maxx ? Right : 0;
        p1_regions |= p1->y < miny ? Bottom : p1->y > maxy ? Top : 0;

        if (p0_regions == Inside && p1_regions == Inside)
            return true;
        else if (p0_regions & p1_regions)
            return false;

        u8  outside = p0_regions ? p0_regions : p1_regions;
        i16 nx = 0, ny = 0;

        if (outside & Top) {
            if (p1->y == p0->y) return false; // Paranoia check
            ny = maxy;
            nx = ((i64)(p1->x - p0->x) * (i64)(maxy - p0->y)) / (p1->y - p0->y) + p0->x;
        } else if (outside & Bottom) {
            if (p1->y == p0->y) return false; // Paranoia check
            ny = miny;
            nx = ((i64)(p1->x - p0->x) * (i64)(miny - p0->y)) / (p1->y - p0->y) + p0->x;
        } else if (outside & Left) {
            if (p1->x == p0->x) return false; // Paranoia check
            nx = minx;
            ny = ((i64)(p1->y - p0->y) * (i64)(minx - p0->x)) / (p1->x - p0->x) + p0->y;
        } else if (outside & Right) {
            if (p1->x == p0->x) return false; // Paranoia check
            nx = maxx;
            ny = ((i64)(p1->y - p0->y) * (i64)(maxx - p0->x)) / (p1->x - p0->x) + p0->y;
        }

        if (outside == p0_regions) {
            p0->x = nx;
            p0->y = ny;
        } else {
            p1->x = nx;
            p1->y = ny;
        }
    }
}

typedef struct {
    u16 stripId;
    u16 vx0Id, vx1Id, vx2Id;
    i32 depth;
} trifx16;

static trifx16 sortedTriangles[VIDEO_MAX_VERTEX_BUFFER_SZ];

int compDepthDesc(const void *a, const void *b)
{
    trifx16 *t0 = a;
    trifx16 *t1 = b;
    if (t0->depth < t1->depth) return 1;
    if (t0->depth > t1->depth) return -1;
    return 0;
}

static vxfx16 vertexBuff[VIDEO_MAX_VERTEX_BUFFER_SZ];
static vxfx16 v2d[VIDEO_MAX_VERTEX_BUFFER_SZ];
static i16v2  vscreen[VIDEO_MAX_VERTEX_BUFFER_SZ];

static void Video_ProcessBatch(TaleaMachine *m, DeviceVideo *v, struct Buff2D *dest, u32 vxbuff,
                               u16 len, u16 focal, fx16v3 woff, i16v3 rotation, fx16 maxDistance,
                               u8 flags)
{
    bool isOriginLinked = flags &
                              (VIDEO_BATCH_TYPE1 | VIDEO_BATCH_TYPE0) == VIDEO_BATCH_TYPE_POINT &&
                          flags & VIDEO_BATCH_MODIFY_TOPOLOGY;

    // preparation:
    // 1. calculate rotations
    double radX, radY, radZ;
    radX = ((double)rotation.x / 32768.0) * PI;
    radY = ((double)rotation.y / 32768.0) * PI;
    radZ = ((double)rotation.z / 32768.0) * PI;

    double sx, sy, sz, cx, cy, cz;
    sx = sin(radX), cx = cos(radX);
    sy = sin(radY), cy = cos(radY);
    sz = sin(radZ), cz = cos(radZ);

    double m0f, m1f, m2f;
    double m3f, m4f, m5f;
    double m6f, m7f, m8f;

    m0f = cy * cz;
    m1f = (sx * sy * cz) - (cx * sz);
    m2f = (cx * sy * cz) + (sx * sz);

    m3f = cy * sz;
    m4f = (sx * sy * sz) + (cx * cz);
    m5f = (cx * sy * sz) - (sx * cz);

    m6f = -sy;
    m7f = sx * cy;
    m8f = cx * cy;

    fx16 m0 = (int)(m0f * 65536);
    fx16 m1 = (int)(m1f * 65536);
    fx16 m2 = (int)(m2f * 65536);
    fx16 m3 = (int)(m3f * 65536);
    fx16 m4 = (int)(m4f * 65536);
    fx16 m5 = (int)(m5f * 65536);
    fx16 m6 = (int)(m6f * 65536);
    fx16 m7 = (int)(m7f * 65536);
    fx16 m8 = (int)(m8f * 65536);

    // 2. clear marker bank
    memset(v->vertexMarkers, 0, sizeof(v->vertexMarkers));
    memset(vertexBuff, 0, sizeof(vertexBuff));
    memset(v2d, 0, sizeof(v2d));

    // Vertex processing:
    for (size_t i = 0; i < len; i++) {
        vxfx16 *vx  = &vertexBuff[i];
        vxfx16 *pvx = &v2d[i];     // projected vertex
        i16v2  *svx = &vscreen[i]; // screen coordinates;

        size_t offset = i * VXI16_REPR_SZ;

        // Fetching and coordinate promotion
        // FIXME: these shoud read physical addresses
        vx->x = TO_FX16(Machine_ReadMain16(m, vxbuff + offset + 0));
        ON_FAULT_RETURN_M
        vx->y = TO_FX16(Machine_ReadMain16(m, vxbuff + offset + 2));
        ON_FAULT_RETURN_M
        vx->z = TO_FX16(Machine_ReadMain16(m, vxbuff + offset + 4));
        ON_FAULT_RETURN_M
        vx->color = Machine_ReadMain8(m, vxbuff + offset + 6);
        vx->flags = Machine_ReadMain8(m, vxbuff + offset + 7);
        ON_FAULT_RETURN_M

        pvx->color = vx->color;
        pvx->flags = vx->flags;
        // Transformation

        if (!(flags & VIDEO_BATCH_ABSOLUTE)) {
            // Rotate the vertex
            fx16 rx, ry, rz;
            rx = ((i64)vx->x * m0 + (i64)vx->y * m1 + (i64)vx->z * m2) >> FX16;
            ry = ((i64)vx->x * m3 + (i64)vx->y * m4 + (i64)vx->z * m5) >> FX16;
            rz = ((i64)vx->x * m6 + (i64)vx->y * m7 + (i64)vx->z * m8) >> FX16;

            if (isOriginLinked) {
                pvx->x = rx + v2d[0].x;
                pvx->y = ry + v2d[0].y;
                pvx->z = rz + v2d[0].z;
            } else {
                // Translate to world offset IF not absolute and not origin linked
                pvx->x = rx + woff.x;
                pvx->y = ry + woff.y;
                pvx->z = rz + woff.z;
            }
        } else {
            if (isOriginLinked) {
                pvx->x = vx->x + v2d[0].x;
                pvx->y = vx->y + v2d[0].y;
                pvx->z = vx->z + v2d[0].z;
            } else {
                pvx->x = vx->x;
                pvx->y = vx->y;
                pvx->z = vx->z;
            }
        }
        /*
                if (i == 0) {
                    TALEA_LOG_TRACE("V0: World X: %f, Y: %f, Z: %f, Flags: %d\n", (float)pvx->x /
           65536.0, (float)pvx->y / 65536.0, (float)pvx->z / 65536.0, pvx->flags);
                }
        */

        // Projection

        if (pvx->z < TO_FX16(10)) { // near z clipping
            pvx->flags |= VIDEO_VX_HIDE;
            continue;
        } else if (pvx->z > maxDistance) {
            pvx->flags |= VIDEO_VX_HIDE;
            continue;
        } else if (flags & VIDEO_BATCH_PERSPECTIVE) {
            pvx->x = ((((i64)pvx->x * (i64)focal) << FX16) / (pvx->z));
            pvx->y = ((((i64)pvx->y * (i64)focal) << FX16) / (pvx->z));

            if (ABS(FROM_FX16(pvx->x)) > (i64)dest->w * 2 ||
                ABS(FROM_FX16(pvx->y)) > (i64)dest->h * 2) {
                pvx->flags |= VIDEO_VX_HIDE;
                continue;
            }
        }

        // Scaling and centering
        int rawSx = (dest->w / 2) + FROM_FX16(pvx->x);
        int rawSy = (dest->h / 2) - FROM_FX16(pvx->y);

        // Safe clamp to avoid i16 wrap-around glitches
        if (rawSx < -32000) rawSx = -32000;
        if (rawSx > 32000) rawSx = 32000;
        if (rawSy < -32000) rawSy = -32000;
        if (rawSy > 32000) rawSy = 32000;

        svx->x = rawSx;
        svx->y = rawSy;
        /*
        if (i == 0) {
            TALEA_LOG_TRACE("Projected: X: %f, Y: %f\n", (float)pvx->x / 65536.0,
                            (float)pvx->y / 65536.0);
            TALEA_LOG_TRACE("Screen: X: %d, Y: %d\n", rawSx, rawSy);
        }
         */

        // Marker handling
        if (pvx->flags & VIDEO_VX_MARK) {
            u8 id                  = pvx->flags >> 4;
            v->vertexMarkers[id].x = pvx->x;
            v->vertexMarkers[id].y = pvx->y;
            v->vertexMarkers[id].z = pvx->z;
        }
    }

    enum VideoBatchType primitive = flags & (VIDEO_BATCH_TYPE1 | VIDEO_BATCH_TYPE0);
    switch (primitive) {
    case VIDEO_BATCH_TYPE_POINT:
        /* points: clipping, z-near check*/
        /* calculate Z-shading if enabled */
        /* how to handle dithering? */
        /* raster with Buff2D_SetPixel() */

        for (size_t i = 0; i < len; i++) {
            vxfx16 *pvx = &v2d[i]; // projected vertex
            i16v2  *svx = &vscreen[i];

            if (pvx->flags & VIDEO_VX_HIDE) continue;
            if (svx->x < 0 || svx->x >= (i64)dest->w) continue;
            if (svx->y < 0 || svx->y >= (i64)dest->h) continue;

            u8 final_color = pvx->color;
            if ((flags & VIDEO_BATCH_ZSHADING) && !(pvx->flags & VIDEO_VX_BRIGHT)) {
                i8 shade    = pvx->z >= maxDistance ? 0 :
                              pvx->z <= 0           ? 15 :
                                                      15 - (i16)(pvx->z << 4) / maxDistance;
                shade       = shade < 0 ? 0 : shade > 15 ? 15 : shade;
                final_color = v->colorShades[pvx->color][shade];
            }

            Buff2D_SetPixel(m, v, dest, final_color, svx->x, svx->y);
        }

        break;
    case VIDEO_BATCH_TYPE_LINE:
        /* lines: clipping, z-near check*/
        /* calculate Z-shading if enabled */
        /* how to handle dithering? */
        /* raster with Bresehnhan */

        bool is_star = flags & VIDEO_BATCH_MODIFY_TOPOLOGY;
        for (size_t i = 0; i < len - 1; i++) {
            size_t  v0id = is_star ? 0 : i;
            vxfx16 *pvx0 = &v2d[v0id];  // projected vertex
            vxfx16 *pvx1 = &v2d[i + 1]; // projected vertex

            if (pvx0->flags & VIDEO_VX_HIDE) continue;
            if (pvx1->flags & VIDEO_VX_HIDE) continue;
            if (!is_star && pvx0->flags & VIDEO_VX_END_OF_STRIP) continue;

            i16v2 svx[2];
            svx[0] = vscreen[v0id];
            svx[1] = vscreen[i + 1];

            bool visible = CohenSutherland(dest, &svx[0], &svx[1]);
            if (!visible) continue;

            u8 final_color = (is_star) ? pvx1->color : pvx0->color;
            if ((flags & VIDEO_BATCH_ZSHADING) && !(pvx0->flags & VIDEO_VX_BRIGHT)) {
                i8 shade    = pvx0->z >= maxDistance ? 0 :
                              pvx0->z <= 0           ? 15 :
                                                       15 - (i16)(pvx0->z << 4) / maxDistance;
                shade       = shade < 0 ? 0 : shade > 15 ? 15 : shade;
                final_color = v->colorShades[pvx0->color][shade];
            }

            Video_DrawLine(m, v, dest, final_color, svx[0].x, svx[0].y, svx[1].x, svx[1].y);
        }
        break;
    case VIDEO_BATCH_TYPE_TRISTRIP:
    case VIDEO_BATCH_TYPE_TRILIST: {
        /* tris: clipping, z-near check*/
        /* culling if not disabled */
        /* calculate Z-shading if enabled */
        /* how to handle dithering? */
        /* raster with DrawTri, probably we have to update it */

        bool   is_fan      = flags & VIDEO_BATCH_MODIFY_TOPOLOGY;
        size_t n_triangles = 0;
        memset(sortedTriangles, 0, sizeof(sortedTriangles));

        size_t step = (primitive == VIDEO_BATCH_TYPE_TRILIST) ? 3 : 1;

        for (size_t i = 0; i < len - 2; i += step) {
            size_t  v0id = is_fan ? 0 : i;
            vxfx16 *pvx0 = &v2d[v0id];  // projected vertex
            vxfx16 *pvx1 = &v2d[i + 1]; // projected vertex
            vxfx16 *pvx2 = &v2d[i + 2]; // projected vertex

            if (primitive == VIDEO_BATCH_TYPE_TRISTRIP && !is_fan &&
                (pvx0->flags & VIDEO_VX_END_OF_STRIP || pvx1->flags & VIDEO_VX_END_OF_STRIP))
                continue;

            sortedTriangles[n_triangles].stripId = i;
            sortedTriangles[n_triangles].vx0Id   = v0id;
            sortedTriangles[n_triangles].vx1Id   = i + 1;
            sortedTriangles[n_triangles].vx2Id   = i + 2;
            sortedTriangles[n_triangles].depth   = (((i64)pvx0->z + pvx1->z + pvx2->z) / 3);

            n_triangles++;
        }

        if (flags & VIDEO_BATCH_DEPTH_SORT) {
            qsort(sortedTriangles, n_triangles, sizeof(trifx16), compDepthDesc);
        }

        for (size_t i = 0; i < n_triangles; i++) {
            trifx16 *tri  = &sortedTriangles[i];
            vxfx16  *pvx0 = &v2d[tri->vx0Id]; // projected vertex
            vxfx16  *pvx1 = &v2d[tri->vx1Id]; // projected vertex
            vxfx16  *pvx2 = &v2d[tri->vx2Id]; // projected vertex

            if (pvx0->flags & VIDEO_VX_HIDE || pvx1->flags & VIDEO_VX_HIDE ||
                pvx2->flags & VIDEO_VX_HIDE) {
                // TALEA_LOG_TRACE("Hiding tri because flag said so\n");
                continue;
            }

            i16v2 *svx0 = &vscreen[tri->vx0Id];
            i16v2 *svx1 = &vscreen[tri->vx1Id];
            i16v2 *svx2 = &vscreen[tri->vx2Id];

            i64 det = (i64)(svx1->x - svx0->x) * (svx2->y - svx0->y) -
                      (i64)(svx1->y - svx0->y) * (svx2->x - svx0->x);

            if (primitive == VIDEO_BATCH_TYPE_TRISTRIP && !is_fan && (tri->stripId % 2 != 0))
                det = -det;

            if (flags & VIDEO_BATCH_BACKFACE_CULLING && det > 0) continue;

            u8 final_shade = pvx0->color;

            if ((flags & VIDEO_BATCH_ZSHADING) && !(pvx0->flags & VIDEO_VX_BRIGHT)) {
                i32 avg_z = tri->depth;
                i8  shade = avg_z >= maxDistance ? 0 :
                            avg_z <= 0           ? 15 :
                                                   15 - (i16)(avg_z << 4) / maxDistance;
                shade     = shade < 0 ? 0 : shade > 15 ? 15 : shade;

                final_shade = v->colorShades[pvx0->color][shade];
            }

            // if (flags & VIDEO_BATCH_DITHER) NOT IMPLEMENTED
            Video_DrawTri(m, v, dest, final_shade, TO_FX16(svx0->x), TO_FX16(svx0->y),
                          TO_FX16(svx1->x), TO_FX16(svx1->y), TO_FX16(svx2->x), TO_FX16(svx2->y));
        }

        break;
    }

    default:
        TALEA_LOG_ERROR("Error in batch, invalid batch type: %d\n", primitive);
        v->error = VIDEO_ERROR_INVALID_BATCH_TYPE;
        return;
    }
}

static void Video_ExecuteCommand(TaleaMachine *m, DeviceVideo *v, struct VideoCmd *cmd)
{
    switch (cmd->op) {
    case VIDEO_COMMAND_CLEAR: {
        // TODO: Document
        // Takes 1 word on GPU0-3 for the pattern to clear the textbuffer
        // Takes 1 byte on GPU4 for the color to clear the framebuffer
        // Takes 1 word on GPU5 for a bitmask on wether to clear the framebuffer ore the textbuffer:
        //      bit 0 (0<<1) to clear textbuffer
        //      bit 1 (1<<1) to clear framebuffer
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
        u8  flags   = cmd->args[0] >> 16;
        Video_Clear(m, v, pattern, color, flags);
    } break;
    case VIDEO_COMMAND_SET_MODE: {
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
        Video_SetMode(m, v, mode);

        break;
    }
    case VIDEO_COMMAND_SET_ADDR: {
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

        if (fb < 0xffffff) {
            v->framebuffer.view =
                Bus_GetView(m, fb, v->framebuffer.w * v->framebuffer.h * v->framebuffer.stride,
                            BUS_ACCESS_READ | BUS_ACCESS_WRITE);
            if (!v->framebuffer.view.ptr ||
                v->framebuffer.view.length !=
                    v->framebuffer.w * v->framebuffer.h * v->framebuffer.stride) {
                TALEA_LOG_WARNING(
                    "Could not set address for framebuffer at 0x%06x (required %d bytes, %d bpp), dma denied (max mem: 0x%08x)\n",
                    fb, 
                    v->framebuffer.w * v->framebuffer.h * v->framebuffer.stride,
                    v->framebuffer.stride,
                    TALEA_MAIN_MEM_SZ
                    );
                v->error = VIDEO_ERROR_DMA;
            }
        }

        if (tb < 0xffffff) {
            v->textbuffer.view =
                Bus_GetView(m, tb, v->textbuffer.w * v->textbuffer.h * v->textbuffer.stride,
                            BUS_ACCESS_READ | BUS_ACCESS_WRITE);
            if (!v->textbuffer.view.ptr || v->textbuffer.view.length != v->textbuffer.w *
                                                                            v->textbuffer.h *
                                                                            v->textbuffer.stride) {
                TALEA_LOG_WARNING("Could not set address for textbuffer, dma denied\n");
                v->error = VIDEO_ERROR_DMA;
            }
        }

        break;
    }
    case VIDEO_COMMAND_LOAD:
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the type of load
        //      0: FONT BASE CP0 2: FONT BASE CP1...ALT FONT CP1 4: PALETTE
        //  Takes 1 half word in GPU1-2 for the address of the buffer in DATA MEMORY
        //  Takes 1 byte in GPU3 for the element count (COLORS or GLYPHS) - 1 (range 1 to 256)
        //  Takes 1 byte in GPU4 for the starting element index (range 0 to 255)
        //  FOR FONT ONLY:
        //  Takes 1 byte in GPU5 for the default glyph index (range 0 to 255) will be converted to
        //      default - starting. Failure if default < starting
        //  Takes 1 byte in GPU6 for the pixel widht of the glyph.
        //      MUST BE == TO BASE CP0 otherwhise fail
        //  Takes 1 byte in GPU7 for the pixel height of the glyph.
        //      MUST BE == TO BASE CP0 otherwhise fail

        if (cmd->argc > 1) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 1) {
            goto not_enough_args;
            break;
        }

        u8     type          = cmd->args[0] >> 56;
        u16    src_addr      = cmd->args[0] >> 40;
        size_t count         = (cmd->args[0] >> 32 & 0xff) + 1;
        u8     start_idx     = cmd->args[0] >> 24;
        u8     default_glyph = cmd->args[0] >> 16;
        u8     cw            = cmd->args[0] >> 8;
        u8     ch            = cmd->args[0];

        if (type < VIDEO_ALT_FONT_CP1 + 1 && default_glyph < start_idx) {
            TALEA_LOG_TRACE("Attempted to load a font with non existing default glyph!\n");
            v->error = VIDEO_ERROR_LOAD;
            break;
        } else {
            default_glyph -= start_idx;
        }

        switch (type) {
        case VIDEO_FONT_BASE_CP1:
        case VIDEO_ALT_FONT_CP1:
        case VIDEO_ALT_FONT_CP0:
            if (v->font.charH != ch || v->font.charW != cw) {
                TALEA_LOG_TRACE(
                    "Attepted to load secondary font slot with different sizes than BASE CP0\n");
                v->error = VIDEO_ERROR_LOAD;
                break;
            }

        case VIDEO_FONT_BASE_CP0: // maybe this is a bit ugly, but less redundant
        {
            size_t bytesPerRow   = ((u32)cw + 7) / 8;
            size_t bytesPerGlyph = bytesPerRow * ch;
            u8    *data          = Bus_GetDataPointer(m, src_addr, bytesPerGlyph * count);
            if (!data) {
                TALEA_LOG_TRACE("Failed to get pointer to font in data memory\n");
                v->error = VIDEO_ERROR_DMA;
                break;
            }

            if (!Video_PrepareFontFromMemory(m, v, type, data, cw, ch, count, start_idx,
                                             default_glyph)) {
                TALEA_LOG_TRACE("Failure to load font from memory in slot: %d\n", type);
                v->error = VIDEO_ERROR_LOAD;
                break;
            } else {
                TALEA_LOG_TRACE("Successfully loaded font from memory in slot: %d\n", type);
            }

            break;
        }
        case VIDEO_ALT_FONT_CP1 + 1: /*Pallete*/
            u8 *data = Bus_GetDataPointer(m, src_addr, 4 * count);
            if (!data) {
                TALEA_LOG_TRACE("Failed to get pointer to pallette in data memory\n");
                v->error = VIDEO_ERROR_DMA;
                break;
            }

            for (size_t i = start_idx, k = 0; i < count + start_idx; i++) {
                v->renderer.shaderPalette[(i * 4)]     = data[(k * 4)];
                v->renderer.shaderPalette[(i * 4) + 1] = data[(k * 4) + 1];
                v->renderer.shaderPalette[(i * 4) + 2] = data[(k * 4) + 2];
                v->renderer.shaderPalette[(i * 4) + 3] = data[(k * 4) + 3];
            }

            Video_BuildShadesTable(v->colorShades, v->renderer.shaderPalette, 256);

            // And set shader uniforms!
            SetShaderValueV(v->renderer.fbShader, v->renderer.fbPaletteLoc,
                            v->renderer.shaderPalette, SHADER_UNIFORM_IVEC4, 256);
            SetShaderValueV(v->renderer.shader, v->renderer.paletteLoc, v->renderer.shaderPalette,
                            SHADER_UNIFORM_IVEC4, 256);
// sadly dont say anything, because if its used for animation printing would be hell
#ifdef TALEA_DEBUG_PALETTE_LOAD
            TALEA_LOG_TRACE("Successfuly loaded %d colors to the pallete\n", count);
#endif

            break;
        default:
            TALEA_LOG_TRACE("Unknonw load subcommand\n");
            v->error = VIDEO_ERROR_LOAD;
            break;
        }

        break;
    case VIDEO_COMMAND_BLIT: {
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

        u16 w = cmd->args[0] >> 16;
        u16 h = cmd->args[0];

        u16 x = cmd->args[1] >> 48;
        u16 y = cmd->args[1] >> 32;

        TaleaMemoryView src_view = Bus_GetView(m, src_addr, (w * h), BUS_ACCESS_READ);
        if (!src_view.ptr || src_view.length != (w * h)) {
            TALEA_LOG_ERROR("COMMAND BLIT could not acquire a view of the destination\n");
            return;
        }
        u8 *src = src_view.ptr;

        enum VideoSpriteRotation rotation = (cmd->args[1] >> 24) & 0x7;

        Video_Blit(m, v, dest, src, w, h, x, y, w, h, rotation);

        break;
    }
    case VIDEO_COMMAND_STRETCH_BLIT: {
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

        u16 w = cmd->args[0] >> 16;
        u16 h = cmd->args[0];

        u16 x      = cmd->args[1] >> 48;
        u16 y      = cmd->args[1] >> 32;
        u16 dest_w = cmd->args[1] >> 16;
        u16 dest_h = cmd->args[1];

        TaleaMemoryView src_view = Bus_GetView(m, src_addr, (w * h), BUS_ACCESS_READ);
        if (!src_view.ptr || src_view.length != (w * h)) {
            TALEA_LOG_ERROR("COMMAND STRETCH BLIT could not acquire a view of the source\n");
            return;
        }
        u8 *src = &src_view.ptr;

        Video_Blit(m, v, dest, src, w, h, x, y, dest_w, dest_h, rotation);

        break;
    }

    case VIDEO_COMMAND_PATTERN_FILL: {
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

        u8 w     = cmd->args[0] >> 24;
        u8 h     = cmd->args[0] >> 16;
        u8 u_off = cmd->args[0] >> 8;
        u8 v_off = cmd->args[0];

        u16 x      = cmd->args[1] >> 48;
        u16 y      = cmd->args[1] >> 32;
        u16 dest_w = cmd->args[1] >> 16;
        u16 dest_h = cmd->args[1];

        TaleaMemoryView src_view = Bus_GetView(m, src_addr, (w * h), BUS_ACCESS_READ);
        if (!src_view.ptr || src_view.length != (w * h)) {
            TALEA_LOG_ERROR("COMMAND PATTERN FILL could not acquire a view of the source\n");
            return;
        }
        u8 *src = &src_view.ptr;

        Video_PatternFill(m, v, dest, src, w, h, u_off, v_off, x, y, dest_w, dest_h, rotation);

        break;
    }

    // GPU commands (queued)
    case VIDEO_COMMAND_DRAW_RECT: {
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

        // TALEA_LOG_TRACE("Drawing Rect color: %d, h: %d, w: %d, x: %d, y: %d\n", color, h, w, x,
        // y);
        Video_DrawRect(m, v, dest, color, x, y, w, h);

        break;
    }
    case VIDEO_COMMAND_DRAW_LINE: {
        // TODO: Document
        // Arg 0:
        //  Takes 1 byte in GPU0 for the destination context pointer
        //  Takes 1 byte GPU2 for the color of the line
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

        // TALEA_LOG_TRACE("Drawing line color: %d, x0: %d, y0: %d, x1: %d, y1: %d\n", color, x0,
        // y0,
        //                x1, y1);
        Video_DrawLine(m, v, dest, color, x0, y0, x1, y1);

        break;
    }
    case VIDEO_COMMAND_DRAW_CIRCLE: {
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

        u8 colorLine = (cmd->args[0] >> 48) & 0xff;
        u8 colorFill = (cmd->args[0] >> 40) & 0xff;
        u8 mode      = (cmd->args[0] >> 32) & 0xff;

        u16 xm = cmd->args[0] >> 16;
        u16 ym = cmd->args[0];

        u16 r = cmd->args[1] >> 48;

        // TALEA_LOG_TRACE("Drawing circle colorLine: %d, colorFill: %d, xm: %d, ym: %d, r: %d\n",
        //                colorLine, colorFill, xm, ym, r);
        Video_DrawCircle(m, v, dest, mode, colorLine, colorFill, xm, ym, r);

        break;
    }
    case VIDEO_COMMAND_DRAW_TRI: {
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

        // CE("Drawing Tri color: %d, x0: %d, y0: %d, x1: %d, y1: %d, x2: %d, y2:
        // %d\n",
        //                 color, x0, y0, x1, y1, x2, y2);
        Video_DrawTri(m, v, dest, color, TO_FX16(x0), TO_FX16(y0), TO_FX16(x1), TO_FX16(y1),
                      TO_FX16(x2), TO_FX16(y2));

        break;
    }
    case VIDEO_COMMAND_DRAW_BATCH: {
        // TODO: Document
        /**
         * Arg0:
         * Takes 1 byte in GPU0 for the destination context pointer
         * Takes 1 sesqui in GPU1-3 for the address of the vertex buffer
         * Takes 1 half word on GPU4-5 for the length of the buffer IN VERTICES
         * Takes 1 half word on GPU6-7 for the focal lenght
         * Arg1:
         * Takes 1 word in GPU0-3 for the world offset x as fx16
         * Takes 1 word in GPU4-7 for the world offset y as fx16
         * Arg2:
         * Takes 1 word in GPU0-3 for the world offset z as fx16
         * Takes 1 half word in GPU4-5 for the rotation x
         * Takes 1 half word in GPU6-7 for the rotation y
         * Arg3:
         * Takes 1 half word in GPU0-1 for the rotation z
         * Takes 1 half word in GPU2-3 for the max distance in z shading (upscaled later to fx16)
         * Takes 1 byte in GPU4 for flags
         */

        if (cmd->argc > 4) {
            goto too_many_args;
            break;
        } else if (cmd->argc < 4) {
            goto not_enough_args;
            break;
        }

        // Arg 0:
        u8  ctx    = cmd->args[0] >> 56;
        u32 vxbuff = cmd->args[0] >> 32 & 0xffffff;
        u16 len    = cmd->args[0] >> 16;
        u16 f      = cmd->args[0];

        struct Buff2D *dest = ctx ? &v->ctx[ctx] : &v->framebuffer;

        // Arg 1:

        fx16v3 woff = { 0 };

        woff.x = cmd->args[1] >> 32;
        woff.y = cmd->args[1];

        // Arg 2:
        woff.z = cmd->args[2] >> 32;

        i16v3 rotation = { 0 };
        rotation.x     = cmd->args[2] >> 16;
        rotation.y     = cmd->args[2];

        // Arg 3:
        rotation.z       = cmd->args[3] >> 48;
        fx16 maxDistance = TO_FX16((cmd->args[3] >> 32) & 0xffff);
        u8   flags       = cmd->args[3] >> 24;

        Video_ProcessBatch(m, v, dest, vxbuff, len, f, woff, rotation, maxDistance, flags);

        break;
    }
    case VIDEO_COMMAND_FILL_SPAN: {
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
    case VIDEO_COMMAND_FILL_VSPAN: {
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
    case VIDEO_COMMAND_SET_CSR: {
        // TODO: Document
        // Takes 1 byte on GPU0 for the csr.
        u8 value = cmd->args[0] >> 56;

        v->vblankEnable = value & VIDEO_VBLANK_EN;
        v->cursorEnable = (value & VIDEO_CURSOR_EN) >> 2;
        v->cursorBlink  = (value & VIDEO_CURSOR_BLINK) >> 3;
        v->rop          = (value & (VIDEO_ROP2 | VIDEO_ROP1 | VIDEO_ROP0)) >> 4;

        v->csr = ASSEMBLE_CSR(v);

        break;
    }
    default: TALEA_LOG_ERROR("Unknown video command, cmd: %d\n", cmd->op); break;
    }
    v->lastExecutedCommand = cmd->op;
    return;

too_many_args:
    TALEA_LOG_WARNING("Video, too many args: %d for command: %d\n", cmd->argc, cmd->op);
    v->error = VIDEO_TOO_MANY_ARGS;
    return;
not_enough_args:
    TALEA_LOG_WARNING("Video, not enough args: %d for command: %d\n", cmd->argc, cmd->op);
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

    BeginTextureMode(*m->video.renderer.screenTexture);
    ClearBackground(BLANK);

    if (m->video.mode == VIDEO_MODE_GRAPHIC || m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        Framebuffer_Update(m);
    }

    if (m->video.mode == VIDEO_MODE_TEXT_MONO || m->video.mode == VIDEO_MODE_TEXT_COLOR ||
        m->video.mode == VIDEO_MODE_TEXT_AND_GRAPHIC) {
        TextMode_Render(m);
    }

    EndTextureMode();

    if (m->video.vblankEnable) {
        Machine_RaiseInterrupt(m, INT_VIDEO_VBLANK, PRIORITY_VBLANK_INTERRUPT);
    }
}

static void Video_PushCommandQueue(TaleaMachine *m, u8 op, struct VideoCmd *cmd)
{
    DeviceVideo *v = &m->video;

    u8 next_head = (v->cmdQueue.head + 1) % VIDEO_CMD_QUEUE_SIZE;
    if (next_head == v->cmdQueue.tail) {
        v->queueFull = true;
        v->error     = VIDEO_ERROR_QUEUE_FULL;
        v->csr       = ASSEMBLE_CSR(v);
        return;
    }

    // TALEA_LOG_TRACE("Pushing video command: %d, argc: %d\n", op, cmd->argc);

    v->cmdQueue.cmd[v->cmdQueue.head].op   = op;
    v->cmdQueue.cmd[v->cmdQueue.head].argc = cmd->argc;
    memcpy(v->cmdQueue.cmd[v->cmdQueue.head].args, cmd->args, sizeof(cmd->args));

    v->cmdQueue.head = next_head;
}

static enum VideoError Video_ProcessCommand(TaleaMachine *m, u8 value)
{
    // TODO: document
    DeviceVideo *v = &m->video;

    switch (value) {
    case VIDEO_COMMAND_END_DRAWING: v->isDrawing = false; break;
    case VIDEO_COMMAND_BEGIN_DRAWING: v->isDrawing = true; break;

    // Immediate commands
    case VIDEO_COMMAND_SYS_INFO:
        // character w, h in pixels
        v->ports[P_VIDEO_GPU0 & 0xf] = v->font.charW;
        v->ports[P_VIDEO_GPU1 & 0xf] = v->font.charH;
        // gives important info on the mode
        v->ports[P_VIDEO_GPU2 & 0xf] = v->mode;
        v->ports[P_VIDEO_GPU3 & 0xf] = v->textbuffer.stride;
        v->ports[P_VIDEO_GPU4 & 0xf] = v->textbuffer.w;
        v->ports[P_VIDEO_GPU5 & 0xf] = v->textbuffer.h;
        // gives important info on the screen
        v->ports[P_VIDEO_GPU6 & 0xf] = v->framebuffer.w >> 8;
        v->ports[P_VIDEO_GPU7 & 0xf] = v->framebuffer.w;
        v->ports[P_VIDEO_GPU8 & 0xf] = v->framebuffer.h >> 8;
        v->ports[P_VIDEO_GPU9 & 0xf] = v->framebuffer.h;
        v->lastExecutedCommand       = value;
        break;

    case VIDEO_COMMAND_BUFFER_INFO:
        v->ports[P_VIDEO_GPU0 & 0xf] = v->textbuffer.view.guest_addr >> 24;
        v->ports[P_VIDEO_GPU1 & 0xf] = v->textbuffer.view.guest_addr >> 16;
        v->ports[P_VIDEO_GPU2 & 0xf] = v->textbuffer.view.guest_addr >> 8;
        v->ports[P_VIDEO_GPU3 & 0xf] = v->textbuffer.view.guest_addr;

        v->ports[P_VIDEO_GPU4 & 0xf] = v->framebuffer.view.guest_addr >> 24;
        v->ports[P_VIDEO_GPU5 & 0xf] = v->framebuffer.view.guest_addr >> 16;
        v->ports[P_VIDEO_GPU6 & 0xf] = v->framebuffer.view.guest_addr >> 8;
        v->ports[P_VIDEO_GPU7 & 0xf] = v->framebuffer.view.guest_addr;

        v->lastExecutedCommand = value;
        break;

    case VIDEO_COMMAND_BIND_CTX: {
        // TODO: document
        // Binds a 2D buffer (address, w, h) to a context slot
        // Takes 1 byte of context slot in GPU0
        // Takes 1 sesqui of the buffer address in GPU1-3
        // Takes 1 halfword in GPU4-5 for the buffer w
        // Takes 1 halfword in GPU6-7 for the buffer h
        // Cannot bind slot 0!

        if (v->error == VIDEO_ERROR_BIND || v->error == VIDEO_ERROR_DMA) return;

        u8 slot = v->currentCmd.args[0] >> 56;

        if (slot == 0) return;

        u32 addr = (v->currentCmd.args[0] >> 32) & 0x00ffffff;

        u16 w = v->currentCmd.args[0] >> 16;
        u16 h = v->currentCmd.args[0];

        TaleaMemoryView view = Bus_GetView(m, addr, w * h, BUS_ACCESS_READ | BUS_ACCESS_WRITE);
        if (!view.ptr || view.length != w * h) {
            TALEA_LOG_WARNING("Cold not get a view to bind slot %d to addr %06x", slot, addr);
            v->error = VIDEO_ERROR_BIND;
            break;
        }

        v->ctx[slot].view   = view;
        v->ctx[slot].w      = w;
        v->ctx[slot].h      = h;
        v->ctx[slot].stride = 1;

        v->lastExecutedCommand = value;
        break;
    }
    case VIDEO_COMMAND_GET_MARKER: {
        // TODO: document
        // returns the specified marker screen coordinates
        // Takes 1 byte of marker id in GPU0

        u8     id     = v->currentCmd.args[0] >> 56;
        i16v3 *marker = &v->vertexMarkers[id & 0xf];

        v->ports[P_VIDEO_GPU0 & 0xf] = marker->x >> 8;
        v->ports[P_VIDEO_GPU1 & 0xf] = marker->x;

        v->ports[P_VIDEO_GPU2 & 0xf] = marker->y >> 8;
        v->ports[P_VIDEO_GPU3 & 0xf] = marker->y;

        v->ports[P_VIDEO_GPU4 & 0xf] = marker->z >> 8;
        v->ports[P_VIDEO_GPU5 & 0xf] = marker->z;

        v->lastExecutedCommand = value;

        break;
    }

    // check if the target buffer is not the framebuffer
    case VIDEO_COMMAND_BLIT:
    case VIDEO_COMMAND_STRETCH_BLIT:
    case VIDEO_COMMAND_PATTERN_FILL:
    case VIDEO_COMMAND_DRAW_RECT:
    case VIDEO_COMMAND_DRAW_LINE:
    case VIDEO_COMMAND_DRAW_CIRCLE:
    case VIDEO_COMMAND_DRAW_TRI:
    case VIDEO_COMMAND_DRAW_BATCH:
    case VIDEO_COMMAND_FILL_SPAN: {
        // This is very ugly
        u8 ctx = v->currentCmd.args[0] >> 56;
        if (value == VIDEO_COMMAND_STRETCH_BLIT || value == VIDEO_COMMAND_PATTERN_FILL) ctx &= 0x1f;

        // Beware of null pointer dereferences!
        if (ctx != 0) {
            v->currentCmd.op = value;
            if (!v->ctx[ctx].view.ptr) {
                v->currentCmd.argc = 0;
                return VIDEO_ERROR_DMA;
            }
            Video_ExecuteCommand(m, v, &v->currentCmd);
        } else {
            if (!v->framebuffer.view.ptr) {
                v->currentCmd.argc = 0;
                return VIDEO_ERROR_DMA;
            }
            Video_PushCommandQueue(m, value, &v->currentCmd);
        }
        break;
    }

    default: Video_PushCommandQueue(m, value, &v->currentCmd);
    }

    // TODO: doocument that writing ANY command, even not queued resets the argument list
    v->currentCmd.argc = 0;
    return VIDEO_NO_ERROR;
}

void Video_Write(TaleaMachine *m, u8 port, u8 value)
{
    DeviceVideo *v = &m->video;
    switch (port & 0xf) {
    case P_VIDEO_COMMAND: {
        v->error = Video_ProcessCommand(m, value);
        break;
    }
    case P_VIDEO_CUR_X: {
        u32 w = v->textbuffer.w;
        u32 h = v->textbuffer.h;
        if (w == 0) break; // Prevent div by zero

        // Ensure current index is within total bounds before extracting Y
        if (v->cursorCellIndex >= w * h) v->cursorCellIndex = 0;

        u32 y              = v->cursorCellIndex / w;
        u32 new_x          = value % w; // Clamp input X to width
        v->cursorCellIndex = (y * w) + new_x;
        // TALEA_LOG_TRACE("Set cursor X to: %d (Y: %d)\n", value, y);
        break;
    }
    case P_VIDEO_CUR_Y: {
        u32 w = v->textbuffer.w;
        u32 h = v->textbuffer.h;
        if (w == 0) break;

        if (v->cursorCellIndex >= w * h) v->cursorCellIndex = 0;

        u32 x              = v->cursorCellIndex % w;
        u32 new_y          = value % h; // Clamp input Y to height
        v->cursorCellIndex = (new_y * w) + x;
        // TALEA_LOG_TRACE("Set cursor Y to: %d (X: %d)\n", value, x);
        break;
    }
    case P_VIDEO_CSR: {
        v->vblankEnable = value & VIDEO_VBLANK_EN;
        v->cursorEnable = (value & VIDEO_CURSOR_EN) >> 2;
        v->cursorBlink  = (value & VIDEO_CURSOR_BLINK) >> 3;
        v->rop          = (value & (VIDEO_ROP2 | VIDEO_ROP1 | VIDEO_ROP0)) >> 4;

        v->csr = ASSEMBLE_CSR(v);

        if (value & VIDEO_RESET_REGS) {
            memset(v->ports, 0, sizeof(v->ports));
        }

        break;
    }
    case P_VIDEO_GPU7: {
        if (v->isDrawing) {
            // FIXME: Why is this not filling arg with the values, and instead with zeroes?
            u64 arg;
            arg = ((u64)v->ports[P_VIDEO_GPU0 & 0xf]) << 56;
            arg |= ((u64)v->ports[P_VIDEO_GPU1 & 0xf]) << 48;
            arg |= ((u64)v->ports[P_VIDEO_GPU2 & 0xf]) << 40;
            arg |= ((u64)v->ports[P_VIDEO_GPU3 & 0xf]) << 32;
            arg |= ((u64)v->ports[P_VIDEO_GPU4 & 0xf]) << 24;
            arg |= ((u64)v->ports[P_VIDEO_GPU5 & 0xf]) << 16;
            arg |= ((u64)v->ports[P_VIDEO_GPU6 & 0xf]) << 8;
            arg |= value;
            if (v->currentCmd.argc < VIDEO_CMD_MAX_ARGS)
                v->currentCmd.args[v->currentCmd.argc] = arg;
            // TALEA_LOG_TRACE("Queueing arg%d: (%016llx) %016llx\n", v->currentCmd.argc, arg,
            //               v->currentCmd.args[v->currentCmd.argc]);
            memset(&v->ports[P_VIDEO_GPU0 & 0xf], 0, 8);
            if (v->currentCmd.argc < VIDEO_CMD_MAX_ARGS) v->currentCmd.argc++;
        }

        break;
    }
    case P_VIDEO_ERROR: v->error = VIDEO_NO_ERROR; // Writing to error clears it
    default: v->ports[port & 0xf] = value;
    }
}

static u8 pixels[TALEA_SCREEN_HEIGHT * TALEA_SCREEN_WIDTH] = { 0 };

void Video_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    if (!is_restart) m->video = (DeviceVideo){ 0 };

    struct FrontendState *state = Frontend_GetState();

    m->video = (DeviceVideo){
        .mode                = VIDEO_MODE_TEXT_MONO,
        .error               = VIDEO_NO_ERROR,
        .rop                 = VIDEO_CONFIG_ROP_COPY,
        .lastExecutedCommand = VIDEO_COMMAND_NOP,

        .transparency = 0,
        .isDrawing    = false,
        .vblankEnable = false,
        .cursorEnable = true,
        .cursorBlink  = true,
        .queueFull    = false,

        .framebuffer.w      = TALEA_SCREEN_WIDTH,
        .framebuffer.h      = TALEA_SCREEN_HEIGHT,
        .framebuffer.stride = 1,
        .textbuffer.w       = 80,
        .textbuffer.h       = 30,
        .textbuffer.stride  = 4,
        .cursorCellIndex    = 0,
        .renderer =
            (VideoRenderer){
                .framebuffer = LoadRenderTexture(TALEA_SCREEN_WIDTH, TALEA_SCREEN_HEIGHT),
                .pixels =
                    LoadTextureFromImage((Image){ .data    = pixels,
                                                  .width   = TALEA_SCREEN_WIDTH,
                                                  .height  = TALEA_SCREEN_HEIGHT,
                                                  .mipmaps = 1,
                                                  .format  = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE }),
                .backgroundColor = BLACK,
                .foregroundColor = GOLD,
                .screenTexture   = &state->window.screenTexture,
            },
    };

    DeviceVideo *v = &m->video;
    v->csr         = ASSEMBLE_CSR(v);

    memcpy(&m->video.renderer.shaderPalette, Palette_Default_Aurora, sizeof(u32) * 4 * (256));
    Video_BuildShadesTable(&m->video.colorShades, &m->video.renderer.shaderPalette, 256);

    Video_PrepareFont(m, &m->video, VIDEO_FONT_BASE_CP0,
                      TextFormat("%s%s", FONT_PATH, config->hardware_font)); // put a list in toml
    Video_PrepareFont(m, &m->video, VIDEO_FONT_BASE_CP1,
                      TextFormat("%s%s", FONT_PATH, config->hardware_font)); // put a list in toml
    Video_PrepareFont(m, &m->video, VIDEO_ALT_FONT_CP0,
                      TextFormat("%s%s", FONT_PATH, config->hardware_font)); // put a list in toml
    Video_PrepareFont(m, &m->video, VIDEO_ALT_FONT_CP1,
                      TextFormat("%s%s", FONT_PATH, config->hardware_font)); // put a list in toml

    m->video.framebuffer.view =
        Bus_GetView(m, VIDEO_FRAMEBUFFER_ADDR,
                    m->video.framebuffer.w * m->video.framebuffer.h * m->video.framebuffer.stride,
                    BUS_ACCESS_READ | BUS_ACCESS_WRITE);

    if (!m->video.framebuffer.view.ptr ||
        m->video.framebuffer.view.length !=
            m->video.framebuffer.w * m->video.framebuffer.h * m->video.framebuffer.stride) {
        TALEA_LOG_WARNING(
            "Could not acquire memory view at %06x for framebuffer on reset. Consider relocating it\n",
            VIDEO_FRAMEBUFFER_ADDR);
        m->video.error = VIDEO_ERROR_DMA;
    }

    m->video.textbuffer.view =
        Bus_GetView(m, VIDEO_TEXTBUFFER_ADDR,
                    m->video.textbuffer.w * m->video.textbuffer.h * m->video.textbuffer.stride,
                    BUS_ACCESS_READ | BUS_ACCESS_WRITE);
    if (!m->video.textbuffer.view.ptr ||
        m->video.textbuffer.view.length !=
            m->video.textbuffer.w * m->video.textbuffer.h * m->video.textbuffer.stride) {
        TALEA_LOG_WARNING(
            "Could not acquire memory view at %06x for textbuffer on reset. Consider relocating it\n",
            VIDEO_TEXTBUFFER_ADDR);
        m->video.error = VIDEO_ERROR_DMA;
    }

    SetTextureFilter(m->video.renderer.pixels, TEXTURE_FILTER_POINT);
    Video_RendererInit(&m->video, SHADERS_PATH("mode_text_mono.fs"));
}
