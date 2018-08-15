#version 440 core

in vec4 fColour;
in vec3 Pos;

out vec4 outputColor;


uniform int uNumFftBands;
uniform float uFft [512];
uniform float uTime;

float hash(float seed)
{
	return fract(sin(seed)*1235.25);
}

void main() {



// SOUND REACTIVE LOGIC
	//BRIGHTNESS
	float brightness = length(fColour.rgb)/255. ;
	int fftLookup = int(brightness * uNumFftBands);
	fftLookup = clamp(fftLookup, 0, uNumFftBands - 1);
	float soundVal = uFft[fftLookup];


	float rc = hash(uTime+Pos.x); 

	rc = (sin(uTime+ Pos.x)*15.25);

	outputColor.rgba = vec4(1.);
	//outputColor.rgb += rc*0.02;

}