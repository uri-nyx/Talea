#version 300 es
precision mediump float;

// Input vertex attributes (from vertex shader)
in vec3 vertexPos;
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform ivec4 baseColor;
uniform int cursorChar;
uniform sampler2D texture0; // screen
uniform sampler2D font; // font
uniform sampler2D characters; // characters 
uniform ivec4 palette[16]; // color palette
uniform ivec4 paletteBg[8]; // color palette
uniform vec2 charSize; // charsize w, h
uniform float uTime;
uniform ivec2 mainTextureSize;

out vec4 finalColor;
void main()
{
    ivec2 charTextureSize = textureSize(characters, 0);
    ivec2 fontTextureSize = textureSize(font, 0);

    // how are distributed chars in the atlas?
    vec2 atlasCharSize = (vec2(fontTextureSize) / vec2(charSize));

    vec2 charTexelSize = 1.0 / vec2(charTextureSize);
    vec2 mainTexelSize = 1.0 / vec2(mainTextureSize);
    
    // Map normalized coordintaes for each character from fragCoord
    vec2 scale = (vec2(mainTextureSize)  / vec2(charTextureSize));
    vec2 charsTexCoord = (fragTexCoord / mainTexelSize) / scale;
    vec2 charUV = mod(charsTexCoord, 1.0) / atlasCharSize;


    // extract character
    vec4 characterData = texture(characters,  fragTexCoord) * 256.0;
    float character = characterData.r;

    vec4 fgColor = vec4(palette[uint(characterData.a) & 0xfu]) / 256.0;
    vec4 bgColor = vec4(paletteBg[(uint(characterData.a) & 0x70u) >> 4]) / 256.0;
    float blink = ((uint(characterData.a) >> 7) == 1u) ? 0.0 : 1.0; // TODO: change api so 1 is NOT blinking

    vec2 charIndexUV = vec2(
        floor(mod(character, floor(atlasCharSize.x))),
        floor(character / floor(atlasCharSize.x))
    );

    charIndexUV /= atlasCharSize;

    // Render font
    vec4 fontRead = texture(font, charIndexUV + charUV);
    finalColor = vec4(1, 1, 1, 1);
/*
    if (fontRead.a == 0.0) {
        finalColor = vec4(mix(bgColor.rgb, vec3(1.0, 0.6875, 0), 0.3), 0.3);
    } else {
        finalColor = vec4(mix(fgColor.rgb, vec3(1.0, 0.796895, 0), 0.23), max(blink, abs(sin(uTime))));
    }
*/
}