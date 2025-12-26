#version 330

in vec3 vertexPos;
in vec2 fragTexCoord;
in vec4 fragColor;

uniform float iTime;
uniform sampler2D texture0;
uniform sampler2D texture1;

out vec4 finalColor;

/*
    CRT pass
*/

//RGB Mask intensity(0 to 1)
#define MASK_INTENSITY 0.3
//Mask size (in pixels)
#define MASK_SIZE 1.2
//Border intensity (0 to 1)
#define MASK_BORDER 0.01

//Chromatic abberration offset in texels (0 = no aberration)
#define ABERRATION_OFFSET vec2(0.013,0.01)

//Curvature intensity
#define SCREEN_CURVATURE 0.045
//Screen vignette
#define SCREEN_VIGNETTE 0.2

//Intensity of pulsing animation
#define PULSE_INTENSITY 0.0606


//Pulse width in pixels (times tau)
#define PULSE_WIDTH 6e1
//Pulse animation speed
#define PULSE_RATE 2e1

void main()
{
    //Resolution
	vec2 res =  vec2(textureSize(texture0, 0));
    //Signed uv coordinates (ranging from -1 to +1)
	vec2 uv = fragTexCoord * 2.0 - 1.0;
    //Scale inward using the square of the distance
	uv *= 1.0 + (dot(uv,uv) - 1.0) * SCREEN_CURVATURE;
    //Convert back to pixel coordinates
	vec2 pixel = (uv*0.5+0.5)*res;
    
    //Square distance to the edge
    vec2 edge = max(1.0 - uv*uv, 0.0);
    //Compute vignette from x/y edges
    float vignette = pow(edge.x * edge.y, SCREEN_VIGNETTE);
	
    //RGB cell and subcell coordinates
    vec2 coord = pixel / MASK_SIZE;
    vec2 subcoord = coord * vec2(3,1);
    //Offset for staggering every other cell
	vec2 cell_offset = vec2(0, fract(floor(coord.x)*0.5));
    
    //Pixel coordinates rounded to the nearest cell
    vec2 mask_coord = floor(coord+cell_offset) * MASK_SIZE;
    
    //Chromatic aberration
	vec4 aberration = texture(texture1, (mask_coord-ABERRATION_OFFSET) / res);
    //Color shift the green channel
	aberration.g = texture(texture1,    (mask_coord+ABERRATION_OFFSET) / res).g;
   
    //Output color with chromatic aberration
	vec4 color = aberration;
    
    //Compute the RGB color index from 0 to 2
    float ind = mod(floor(subcoord.x), 3.0);
    //Convert that value to an RGB color (multiplied to maintain brightness)
    vec3 mask_color = vec3(ind == 0.0, ind == 1.0, ind == 2.0) * 3.0;
    
    //Signed subcell uvs (ranging from -1 to +1)
    vec2 cell_uv = fract(subcoord + cell_offset) * 2.0 - 1.0;
    //X and y borders
    vec2 border = 1.0 - cell_uv * cell_uv * MASK_BORDER;
    //Blend x and y mask borders
    mask_color.rgb *= border.x * border.y;
    //Blend with color mask
	color.rgb *= 1.0 + (mask_color - 1.0) * MASK_INTENSITY;  
    
    //Apply vignette
    color.rgb *= vignette;
    //Apply pulsing glow
	color.rgb *= 1.0+PULSE_INTENSITY*cos(pixel.x/PULSE_WIDTH+iTime*PULSE_RATE);
    
    finalColor = color;
}

/*

vec2 curve(vec2 uv)
{
	uv = (uv - 0.5) * 2.0;
	uv *= 1.1;	
	uv.x *= 1.0 + pow((abs(uv.y) / 5.0), 2.0);
	uv.y *= 1.0 + pow((abs(uv.x) / 4.0), 2.0);
	uv  = (uv / 2.0) + 0.5;
	uv =  uv *0.92 + 0.04;
	return uv;
}
void main()
{
    vec2 iResolution = vec2(textureSize(texture0, 0));
    vec2 uv = fragTexCoord;
    uv = curve( uv );
    vec3 oricol = texture( texture1, fragTexCoord ).xyz;
    vec3 col;
	float x =  sin(0.3*iTime+uv.y*21.0)*sin(0.7*iTime+uv.y*29.0)*sin(0.3+0.33*iTime+uv.y*31.0)*0.0017;

    col.r = texture(texture1,vec2(x+uv.x+0.001,uv.y+0.001)).x+0.05;
    col.g = texture(texture1,vec2(x+uv.x+0.000,uv.y-0.002)).y+0.05;
    col.b = texture(texture1,vec2(x+uv.x-0.002,uv.y+0.000)).z+0.05;
    col.r += 0.08*texture(texture1,0.75*vec2(x+0.025, -0.027)+vec2(uv.x+0.001,uv.y+0.001)).x;
    col.g += 0.05*texture(texture1,0.75*vec2(x+-0.022, -0.02)+vec2(uv.x+0.000,uv.y-0.002)).y;
    col.b += 0.08*texture(texture1,0.75*vec2(x+-0.02, -0.018)+vec2(uv.x-0.002,uv.y+0.000)).z;

    col = clamp(col*0.6+0.4*col*col*1.0,0.0,1.0);

    float vig = (0.0 + 1.0*16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y));
	col *= vec3(pow(vig,0.3));

    col *= vec3(0.95,1.05,0.95);
	col *= 2.8;

	float scans = clamp( 0.35+0.35*sin(3.5*iTime+uv.y*iResolution.y*1.5), 0.0, 1.0);
	
	float s = pow(scans,1.7);
	col = col*vec3( 0.4+0.7*s) ;

    col *= 1.0+0.01*sin(110.0*iTime);
	if (uv.x < 0.0 || uv.x > 1.0)
		col *= 0.0;
	if (uv.y < 0.0 || uv.y > 1.0)
		col *= 0.0;
	
	col*=1.0-0.65*vec3(clamp((mod(fragTexCoord.x, 2.0)-1.0)*2.0,0.0,1.0));
	
    float comp = smoothstep( 0.1, 0.9, sin(iTime) );
 
	// Remove the next line to stop cross-fade between original and postprocess
    // col = mix( col, oricol, comp );

    finalColor = vec4(col,1.0);
}
*/