#version 330

// Input vertex attributes (from vertex shader)
in vec3 vertexPos;
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform ivec3 baseColor;
uniform int cursorChar;
uniform float uTime;
uniform sampler2D texture0; // screen
uniform sampler2D font; // font
uniform sampler2D characters; // characters 

out vec4 finalColor;


void main()
{

    ivec2 charTextureSize = textureSize(characters, 0);
    ivec2 mainTextureSize = textureSize(texture0, 0);
    ivec2 fontTextureSize = textureSize(font, 0);

    // how are distributed chars in the atlas?
    ivec2 atlasCharSize = fontTextureSize / ivec2(8, 16);

    vec2 charTexelSize = 1.0 / charTextureSize;
    vec2 mainTexelSize = 1.0 / mainTextureSize;
    
    // Map normalized coordintaes for each character from fragCoord
    vec2 scale = (mainTextureSize  / charTextureSize);
    vec2 charsTexCoord = (fragTexCoord / mainTexelSize) / scale;
    vec2 charUV = mod(charsTexCoord, 1.0) / atlasCharSize;


    // extract character
    vec4 characterData = texture(characters,  fragTexCoord) * 256;
    float character = characterData.r;

    vec2 charIndexUV = vec2(
        floor(mod(character, atlasCharSize.x)),
        floor(character / atlasCharSize.x)
    );

    charIndexUV /= atlasCharSize;

    // Render font
    vec4 fontRead = texture(font, charIndexUV + charUV);

    //if (character == cursorChar) finalColor = vec4(baseColor / 256, fontRead.a * min(1, abs(sin(uTime))));
     
    finalColor = vec4(1, 1, 1, fontRead.a);
}