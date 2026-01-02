#version 330

// Input vertex attributes (from vertex shader)
in vec3 vertexPos;
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform int       cursorIndex;
uniform int       csr; // bitmask: 0x1: ENABLE, 0x2: Blink, 0x4: SHAPE
uniform float     uTime;
uniform sampler2D texture0;   // screen
uniform sampler2D font_cp0;       // font
uniform sampler2D characters; // characters

out vec4 finalColor;

void main()
{
    ivec2 charTextureSize = textureSize(characters, 0);
    ivec2 mainTextureSize = textureSize(texture0, 0);
    ivec2 fontTextureSize = textureSize(font_cp0, 0);

    // how are distributed chars in the atlas?
    ivec2 atlasCharSize = fontTextureSize / ivec2(8, 16);

    vec2 charTexelSize = 1.0 / charTextureSize;
    vec2 mainTexelSize = 1.0 / mainTextureSize;

    // Map normalized coordintaes for each character from fragCoord
    vec2 scale         = (mainTextureSize / charTextureSize);
    vec2 charsTexCoord = (fragTexCoord / mainTexelSize) / scale;
    vec2 charUV        = mod(charsTexCoord, 1.0) / atlasCharSize;

    // extract character
    vec4  characterData = texture(characters, fragTexCoord) * 256;
    float character     = characterData.r;

    vec2 charIndexUV =
        vec2(floor(mod(character, atlasCharSize.x)), floor(character / atlasCharSize.x));

    charIndexUV /= atlasCharSize;

    ivec2 currentCell  = ivec2(fragTexCoord * charTextureSize);
    int   currentIndex = currentCell.y * charTextureSize.x + currentCell.x;

    // Render font
    vec4  fontRead = texture(font_cp0, charIndexUV + charUV);
    float alpha    = fontRead.a;

    // Cursor Overlay Logic. Maybe pass uniform bools of the csr
    if (currentIndex == cursorIndex && ((csr& 1) == 1)) {
        float blinkFactor = 1.0;
        
        if ((csr& 2) == 2)
        {
            // Blink
            blinkFactor = (sin(uTime * 3.14159) * 0.5) + 0.5;
        }

        float invertedAlpha = 1.0 - alpha;
        alpha = mix(alpha, invertedAlpha, blinkFactor);
        // TODO: add different shapes
    }

    finalColor = vec4(1, 1, 1, alpha);
}