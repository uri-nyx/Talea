#version 330

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
uniform float uTime;

uniform ivec2 mainTextureSize;
uniform ivec4 palette[16]; // color palette
uniform ivec4 paletteBg[8]; // color palette
uniform vec2 charSize; // charsize w, h

out vec4 finalColor;

void main()
{

    ivec2 charTextureSize = textureSize(characters, 0);
    //ivec2 mainTextureSize = textureSize(texture0, 0);
    ivec2 fontTextureSize = textureSize(font, 0);

    // how are distributed chars in the atlas?
    vec2 atlasCharSize = fontTextureSize / charSize;

    vec2 charTexelSize = 1.0 / charTextureSize;
    vec2 mainTexelSize = 1.0 / mainTextureSize;
    
    // Map normalized coordintaes for each character from fragCoord
    vec2 scale = (mainTextureSize  / charTextureSize);
    vec2 charsTexCoord = (fragTexCoord / mainTexelSize) / scale;
    vec2 charUV = mod(charsTexCoord, 1.0) / atlasCharSize;


    // extract character
    vec4 characterData = texture(characters,  fragTexCoord) * 256;
    float character = characterData.r;

    vec4 fgColor = vec4(palette[uint(characterData.a) & 0xfu]) / 256;
    vec4 bgColor = vec4(paletteBg[(uint(characterData.a) & 0x70u) >> 4]) / 256;
    bool blink = ((uint(characterData.a) >> 7) == 1u); // TODO: change api so 1 is NOT blinking

    vec2 charIndexUV = vec2(
        floor(mod(character, floor(atlasCharSize.x))),
        floor(character / floor(atlasCharSize.x))
    );

    charIndexUV /= atlasCharSize;

    // Render font
    vec4 fontRead = texture(font, charIndexUV + charUV);

    vec4 bg =  vec4(mix(bgColor.rgb, vec3(1.0, 0.6875, 0), 0.06), 0.2);

    if (fontRead.a == 0) {
        finalColor = bg;
    } else if (blink) {
        finalColor = mix(vec4(mix(fgColor.rgb, vec3(1.0, 0.796895, 0), 0.023), 1), bg, abs(sin(uTime)));
    } else {
        finalColor = vec4(mix(fgColor.rgb, vec3(1.0, 0.796895, 0), 0.023), 1);
    }
}