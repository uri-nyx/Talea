#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D pixels; // The 640x480 grayscale texture
uniform ivec4 palette[256];     // Your existing palette array

void main()
{
    // 1. Sample the index from the 'Red' channel (Grayscale maps to R, G, and B)
    // We multiply by 255 to get the 0-255 integer index
    float indexFloat = texture(pixels, fragTexCoord).r;
    int index = int(indexFloat * 255.0 + 0.5);

    // 2. Palette Lookup
    vec4 color = vec4(palette[index]) / 255.0;

    finalColor = color;
}