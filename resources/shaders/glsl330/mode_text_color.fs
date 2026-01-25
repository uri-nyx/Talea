#version 330

in vec2 fragTexCoord;

uniform sampler2D font_atlas;
uniform sampler2D characters;
uniform ivec4     palette[256];
uniform vec2      charSize;
uniform ivec2     mainTextureSize;
uniform float     uTime;
uniform int       cursorIndex;
uniform int       csr;

out vec4 finalColor;

void main()
{
    // 1. Fetch character data
    // Using round to ensure floating point inaccuracies in the sampler
    // don't shift a 65 (0.2549) to 64.
    vec4  cell      = round(texture(characters, fragTexCoord) * 255.0);
    float character = cell.r;
    uint  attr      = uint(cell.a);

    // 2. Missing Variable Definitions
    // fontColumn: extracted from attribute bits (assuming bits 1-2 based on your previous specs)
    int   fontColumn      = int((attr >> 1) & 3u);
    vec2  textBufferSize  = vec2(mainTextureSize) / charSize;
    ivec2 charTextureSize = textureSize(characters, 0);

    // 3. Coordinate Math
    vec2 screenPixel = fragTexCoord * vec2(mainTextureSize);
    vec2 pixelInCell = mod(screenPixel, charSize);

    // Robust local UV (0.0 to 1.0)
    vec2 localUV = pixelInCell / charSize;

    // 4. Attributes
    bool blink       = (attr & 128u) != 0u;
    bool bold        = (attr & 64u) != 0u;
    bool underline   = (attr & 32u) != 0u;
    bool dim         = (attr & 16u) != 0u;
    bool oblique     = (attr & 8u) != 0u;
    bool transparent = (attr & 4u) != 0u;

    bool inside_bounding_box = true;
    bool isUnderlinePixel    = underline && (localUV.y > 0.85 && localUV.y < 0.92);
    if (oblique) {
        localUV.x += (localUV.y - 0.7) * 0.25;
        if (localUV.x < 0.15 || localUV.x > 1.15) inside_bounding_box = false;
    }

    // 5. Final Atlas Sampling
    // We add 0.5 to the character index and use floor to ensure we
    // land in the dead center of the intended character row.
    vec2 atlasUV = vec2((float(fontColumn) + clamp(localUV.x, 0.0, 1.0)) / 4.0,
                        (256.0 - floor(character) - clamp(localUV.y, 0.0, 1.0)) / 256.0);
    // PSF fonts are 1-bit; Raylib LoadTextureFromImage usually puts data in ALL channels
    float fontRead = texture(font_atlas, atlasUV).r;
    if (!inside_bounding_box) fontRead = 0.0;

    if (bold && fontRead < 0.5) {
        float offX = 0.5 / (4.0 * charSize.x);
        fontRead   = texture(font_atlas, atlasUV - vec2(offX, 0.0)).r;
    }

    // 7. Colors & Output
    vec4 fgColor = vec4(palette[uint(cell.g)]) / 255.0;
    vec4 bgColor = vec4(palette[uint(cell.b)]) / 255.0;
    if (dim) fgColor.rgb *= 0.5;

    if (blink) {
        finalColor.a = abs(sin(uTime));
    }

    vec4 outColor;

    // Cursor
    ivec2 currentCell    = ivec2(fragTexCoord * textBufferSize);
    int   currentIndex   = currentCell.y * charTextureSize.x + currentCell.x;
    bool  cursor_here    = (currentIndex == cursorIndex);
    bool  cursor_enabled = (csr & 4) != 0;
    bool  cursor_blink   = (csr & 8) != 0;

    if (fontRead > 0.5 || isUnderlinePixel) {
        outColor = fgColor;
        if (dim) outColor.rgb *= 0.5; // Apply DIM attribute
    } else {
        if (transparent && !(cursor_enabled && cursor_here)) discard;
        outColor = bgColor;
    }

    if (blink) {
        outColor.a = abs(sin(uTime));
    }

    if (cursor_here && cursor_enabled) {
        float blinkFactor = 1.0;

        if (cursor_blink) {
            // Blink
            blinkFactor = (sin(uTime * 3.14159) * 0.5) + 0.5;
        }

        float invertedAlpha = 1.0 - outColor.a;
        outColor.a          = mix(outColor.a, 1.0, blinkFactor);
        // TODO: add different shapes
        // inverted color
        outColor.rgb = mix(outColor.rgb, 1.0 - outColor.rgb, blinkFactor);
    }

    finalColor = outColor;
}
/*
#version 330

// Input vertex attributes (from vertex shader)
in vec3 vertexPos;
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform int       cursorIndex;
uniform int       csr;      // bitmask: 0x
uniform sampler2D texture0; // screen
uniform sampler2D font_atlas;
uniform sampler2D characters; // characters
uniform float     uTime;

uniform ivec2 mainTextureSize;
uniform ivec4 palette[256]; // color palette
uniform vec2  charSize;     // charsize w, h

out vec4 finalColor;

void main()
{

// extract character information
vec4  characterData = round(texture(characters, fragTexCoord) * 255.0);
float character     = characterData.r;
// Bitmask
bool blink       = ((uint(characterData.a) & 128u) == 128u);
bool bold        = ((uint(characterData.a) & 64u) == 64u);
bool underline   = ((uint(characterData.a) & 32u) == 32u);
bool dim         = ((uint(characterData.a) & 16u) == 16u);
bool oblique     = ((uint(characterData.a) & 8u) == 8u);
bool transparent = ((uint(characterData.a) & 4u) == 4u);
bool alt_font    = ((uint(characterData.a) & 2u) == 2u);
bool codepage    = ((uint(characterData.a) & 1u) == 1u);

uint font_index = (uint(alt_font) * 2u) + uint(codepage);

ivec2 charTextureSize = textureSize(characters, 0);
// ivec2 mainTextureSize = textureSize(texture0, 0);
ivec2 fontTextureSize = getFontTextureSize(font_index);

// how are distributed chars in the atlas?
vec2 atlasCharSize = floor(fontTextureSize / charSize);

vec2 charTexelSize = 1.0 / charTextureSize;
vec2 mainTexelSize = 1.0 / mainTextureSize;

// Map normalized coordintaes for each character from fragCoord
vec2 scale         = (mainTextureSize / charTextureSize);
vec2 charsTexCoord = (fragTexCoord / mainTexelSize) / scale;

vec4 fgColor = vec4(palette[uint(characterData.g)]) / 255.0;
vec4 bgColor = vec4(palette[uint(characterData.b)]) / 255.0;

vec2 charIndexUV = vec2(floor(mod(character, floor(atlasCharSize.x))),
                        floor(character / floor(atlasCharSize.x)));

charIndexUV /= atlasCharSize;

//ivec2 pixelCoord = ivec2(gl_FragCoord.x, mainTextureSize.y - gl_FragCoord.y);

// 2. Determine which character cell this pixel belongs to
// mainTextureSize / charTextureSize gives you the pixels per character (e.g., 8x8)
//ivec2 pixelsPerChar = mainTextureSize / charTextureSize;
//ivec2 currentCell = pixelCoord / pixelsPerChar;

// 3. Calculate linear index
// Use the character grid width (charTextureSize.x)
//int   currentIndex = currentCell.y * charTextureSize.x + currentCell.x;

ivec2 currentCell  = ivec2(fragTexCoord * charTextureSize);
int   currentIndex = currentCell.y * charTextureSize.x + currentCell.x;

vec2 localUV = mod(charsTexCoord, 1.0);
bool inside_bounding_box = true;

bool isUnderlinePixel = underline && (localUV.y > 0.85 && localUV.y < 0.92);
if (oblique) {
    localUV.x += (localUV.y - 0.7) * 0.25;
    if (localUV.x < 0.15 || localUV.x > 1.15) inside_bounding_box = false;
}

vec2 charUV = localUV / atlasCharSize;

// Render font
vec4 fontRead = getFontSample(font_index, charIndexUV + charUV);
if (!inside_bounding_box) fontRead.a = 0.0;

// vec4 bg =  vec4(mix(bgColor.rgb, vec3(1.0, 0.6875, 0), 0.06), 0.2);

if (bold && fontRead.a < 0.5) {
    // Sample one font-texel to the left
    float offX       = 1.0 / float(fontTextureSize.x);
    float boldSample = getFontSample(font_index, (charIndexUV + charUV) - vec2(offX, 0.0)).a;
    fontRead.a       = max(fontRead.a, boldSample);
}

vec4 outColor;

bool cursor_enabled = (csr & 4) != 0;
bool cursor_blink   = (csr & 8) != 0;
bool cursor_here    = currentIndex == cursorIndex;

if (fontRead.a > 0.5 || isUnderlinePixel) {
    outColor = fgColor;
    if (dim) outColor.rgb *= 0.5; // Apply DIM attribute
} else {
    if (transparent && !(cursor_enabled && cursor_here)) discard;
    outColor = bgColor;
}

if (blink) {
    outColor.a = abs(sin(uTime));
}

if (cursor_here && cursor_enabled) {
    float blinkFactor = 1.0;

    if (cursor_blink) {
        // Blink
        blinkFactor = (sin(uTime * 3.14159) * 0.5) + 0.5;
    }

    float invertedAlpha = 1.0 - outColor.a;
    outColor.a              = mix(outColor.a, 1.0, blinkFactor);
    // TODO: add different shapes
    // inverted color
    outColor.rgb = mix(outColor.rgb, 1.0 - outColor.rgb, blinkFactor);;
}

finalColor = outColor;
}
*/