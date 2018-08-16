#version 440 core

in vec4 Pos;
in vec4 fColour;

layout ( location = 0) out vec4 outputColour;
layout ( location = 1) out vec4 brightColour;


uniform int uNumFftBands;
uniform float uFft [512];
uniform float uTime;

uniform float uBloomThreshhold = .9;

float hash(float seed)
{
	return fract(sin(seed)*1235.25);
}

void main() {



// SOUND REACTIVE LOGIC
	//BRIGHTNESS
	float brightness = length(fColour.rgb) ;
	int fftLookup = int(brightness * uNumFftBands);


	outputColour.rgba = vec4(fColour);

    // check whether fragment output is higher than threshold, if so output as brightness color
    if(brightness > uBloomThreshhold)
        brightColour = vec4(outputColour.rgb, 1.0);
    else
        brightColour = vec4(0.0, 0.0, 0.0, 1.0);


}