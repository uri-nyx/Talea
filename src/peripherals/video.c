#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_PIC
#define STBI_ONLY_PNM
#define OLIVEC_LOADFONT
#define STB_IMAGE_IMPLEMENTATION
#define OLIVEC_IMPLEMENTATION
#include "wrapper.h"

Olivec_Font
wrapper_load_font(const char* path, int glyph_w, int glyph_h, int offset)
{
    int w, h, channels;
    unsigned char * atlas = stbi_load(path, &w, &h, &channels, 0);
    return olivec_loadfont_frombuff(atlas, w, h, channels, glyph_w, glyph_h, offset);
}
