#version 330

// Input vertex attributes (from vertex shader)
in vec3 vertexPos;
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform int       cursorIndex;
uniform int       csr; // bitmask: 0x
uniform sampler2D texture0; // screen
uniform sampler2D font_cp0;
uniform sampler2D font_cp1;
uniform sampler2D alt_font_cp0;
uniform sampler2D alt_font_cp1;
uniform sampler2D characters; // characters
uniform float     uTime;

uniform ivec2 mainTextureSize;
uniform ivec4 palette[256]; // color palette
uniform vec2  charSize;     // charsize w, h

out vec4 finalColor;

vec4 getFontSample(uint index, vec2 uv)
{
    if (index == 0u) return texture(font_cp0, uv);
    if (index == 1u) return texture(font_cp1, uv);
    if (index == 2u) return texture(alt_font_cp0, uv);
    return texture(alt_font_cp1, uv);
}

ivec2 getFontTextureSize(uint index)
{
    if (index == 0u) return textureSize(font_cp0, 0);
    if (index == 1u) return textureSize(font_cp1, 0);
    if (index == 2u) return textureSize(alt_font_cp0, 0);
    return textureSize(alt_font_cp1, 0);
}

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

    vec2 localUV = mod(charsTexCoord, 1.0);

    bool isUnderlinePixel = underline && (localUV.y > 0.85 && localUV.y < 0.92);
    if (oblique) {
        localUV.x += (localUV.y - 0.7) * 0.25;
        // if (localUV.x < 0.0 || localUV.x > 1.0) discard;
    }

    vec2 charUV = localUV / atlasCharSize;

    // Render font
    vec4 fontRead = getFontSample(font_index, charIndexUV + charUV);

    // vec4 bg =  vec4(mix(bgColor.rgb, vec3(1.0, 0.6875, 0), 0.06), 0.2);

    if (bold && fontRead.a < 0.5) {
        // Sample one font-texel to the left
        float offX       = 1.0 / float(fontTextureSize.x);
        float boldSample = getFontSample(font_index, (charIndexUV + charUV) - vec2(offX, 0.0)).a;
        fontRead.a       = max(fontRead.a, boldSample);
    }

    vec4 outColor;

    if (fontRead.a > 0.5 || isUnderlinePixel) {
        outColor = fgColor;
        if (dim) outColor.rgb *= 0.5; // Apply DIM attribute
    } else {
        //if (transparent) discard;
        outColor = bgColor;
    }

    if (blink) {
        outColor.a = abs(sin(uTime));
    }

    finalColor = outColor;
}