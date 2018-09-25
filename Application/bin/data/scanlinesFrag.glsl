#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform int uNumFftBands = 512;
uniform float uFft [512];
uniform float uTime = 1.0;
uniform int uXDivisions = 2;
uniform int uYDivisions = 2;

void main()
{             
	vec2 uv = TexCoords.xy;
	uv.x *= uXDivisions;
	uv.y *= uYDivisions;
	uv = fract(uv);


    FragColor = vec4(uv, .0, 1);
}  