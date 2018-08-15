#version 440 core

in vec4 fColour;
in vec3 Pos;

layout ( location = 0) out vec4 outputColour;
layout ( location = 1) out vec4 brightColour;


uniform int uNumFftBands;
uniform float uFft [512];
uniform float uTime;

uniform float uBloomThreshhold = .07;

float hash(float seed)
{
	return fract(sin(seed)*1235.25);
}

void main() {



// SOUND REACTIVE LOGIC
	//BRIGHTNESS
	float brightness = length(fColour.rgb) ;
	int fftLookup = int(brightness * uNumFftBands);
	fftLookup = clamp(fftLookup, 0, uNumFftBands - 1);
	float soundVal = uFft[fftLookup];


	float rc = hash(uTime+Pos.x); 

	rc = (sin(uTime+ Pos.x)*15.25);

	outputColour.rgba = vec4(1.);


    // check whether fragment output is higher than threshold, if so output as brightness color
    if(brightness > uBloomThreshhold)
        brightColour = vec4(outputColour.rgb, 1.0);
    else
        brightColour = vec4(0.0, 0.0, 0.0, 1.0);
	//outputColor.rgb += rc*0.02;

}