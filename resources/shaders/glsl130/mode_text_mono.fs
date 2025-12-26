#version 300 es
precision mediump float;

// Input vertex attributes (from vertex shader)
in vec3 vertexPos;
in vec2 fragTexCoord;
in vec4 fragColor;

// varyingput uniform values
uniform ivec3 baseColor;
uniform int cursorChar;
uniform float uTime;
uniform sampler2D texture0; // screen
uniform sampler2D font; // font
uniform sampler2D characters; // characters 
uniform ivec2 charTextureSize;
uniform ivec2 mainTextureSize;
uniform ivec2 fontTextureSize;

out vec4 finalColor;

void main()
{

    // how are distributed chars in the atlas?
    ivec2 atlasCharSize = fontTextureSize / ivec2(8, 16);

    vec2 charTexelSize = 1.0 / vec2(charTextureSize);
    vec2 mainTexelSize = 1.0 / vec2(mainTextureSize);
    
    // Map normalized coordintaes for each character from fragCoord
    vec2 scale = (vec2(mainTextureSize)  / vec2(charTextureSize));
    vec2 charsTexCoord = (fragTexCoord / mainTexelSize) / scale;
    vec2 charUV = mod(charsTexCoord, 1.0) / vec2(atlasCharSize);


    // extract character
    vec4 characterData = texture(characters,  fragTexCoord) * vec4(256.0);
    float character = characterData.r;

    vec2 charIndexUV = vec2(
        floor(mod(character, float(atlasCharSize.x))),
        floor(character / float(atlasCharSize.x))
    );

    charIndexUV /= vec2(atlasCharSize);

    // Render font
    vec4 fontRead = texture(font, charIndexUV + charUV);

    //if (character == cursorChar) finalColor = vec4(baseColor / 256, fontRead.a * min(1, abs(sin(uTime))));
     
    finalColor = vec4(1, 1, 1, fontRead.a);
}