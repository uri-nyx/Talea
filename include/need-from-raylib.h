#ifndef NEED_FROM_RAYLIB_H
#define NEED_FROM_RAYLIB_H

#define RLAPI

// Color, 4 components, R8G8B8A8 (32bit)
typedef struct Color {
    unsigned char r; // Color red value
    unsigned char g; // Color green value
    unsigned char b; // Color blue value
    unsigned char a; // Color alpha value
} Color;             // Text formatting with variables (sprintf() style)

// Texture, tex data stored in GPU memory (VRAM)
typedef struct Texture {
    unsigned int id;      // OpenGL texture id
    int          width;   // Texture base width
    int          height;  // Texture base height
    int          mipmaps; // Mipmap levels, 1 by default
    int          format;  // Data format (PixelFormat type)
} Texture;

// Texture2D, same as Texture
typedef Texture Texture2D;

// RenderTexture, fbo for texture rendering
typedef struct RenderTexture {
    unsigned int id;      // OpenGL framebuffer object id
    Texture      texture; // Color buffer attachment texture
    Texture      depth;   // Depth buffer attachment texture
} RenderTexture;

// RenderTexture2D, same as RenderTexture
typedef RenderTexture RenderTexture2D;

// Shader
typedef struct Shader {
    unsigned int id;   // Shader program id
    int         *locs; // Shader locations array (RL_MAX_SHADER_LOCATIONS)
} Shader;

// Rectangle, 4 components
typedef struct Rectangle {
    float x;      // Rectangle top-left corner position x
    float y;      // Rectangle top-left corner position y
    float width;  // Rectangle width
    float height; // Rectangle height
} RayRectangle;

// Image, pixel data stored in CPU memory (RAM)
typedef struct Image {
    void *data;    // Image raw data
    int   width;   // Image base width
    int   height;  // Image base height
    int   mipmaps; // Mipmap levels, 1 by default
    int   format;  // Data format (PixelFormat type)
} Image;

// GlyphInfo, font characters glyphs info
typedef struct GlyphInfo {
    int   value;    // Character value (Unicode)
    int   offsetX;  // Character offset X when drawing
    int   offsetY;  // Character offset Y when drawing
    int   advanceX; // Character advance position X
    Image image;    // Character image data
} GlyphInfo;

// Font, font texture and GlyphInfo array data
typedef struct Font {
    int           baseSize;     // Base size (default chars height)
    int           glyphCount;   // Number of glyph characters
    int           glyphPadding; // Padding around the glyph characters
    Texture2D     texture;      // Texture atlas containing the glyphs
    RayRectangle *recs;         // Rectangles in texture for the glyphs
    GlyphInfo    *glyphs;       // Glyphs info data
} Font;

// File path list
typedef struct FilePathList {
    unsigned int capacity; // Filepaths max entries
    unsigned int count;    // Filepaths entries count
    char       **paths;    // Filepaths entries
} FilePathList;

RLAPI const char *TextFormat(const char *text, ...);

#endif /* NEED_FROM_RAYLIB_H */
