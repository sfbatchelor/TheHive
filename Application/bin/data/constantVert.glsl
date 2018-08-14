#version 440 core

// these are for the programmable pipeline system and are passed in
// by default from OpenFrameworks
uniform mat4 modelViewProjectionMatrix;

uniform int uNumFftBands;
uniform float uFft [512];

in vec4 position;
in vec4 colour;

out vec4 vColour;
out float vSoundVal;
out vec3 Pos;

uniform float uAlpha = 1.;

void main()
{

	float brightness =  length(colour.rgb)/255.;
	brightness += 0.5;
	brightness = clamp(brightness, 0, 1);
	brightness = 1. - brightness;
	int fftLookup = int(brightness * uNumFftBands);
	fftLookup = clamp(fftLookup, 0, uNumFftBands - 1);
	vSoundVal = uFft[fftLookup];

	vColour = colour/255.;
	vColour.rgb *= uAlpha;
	vColour.a = uAlpha;


	vec4 pos = position;
	pos.z += 1*vSoundVal;
	gl_Position = pos;
	Pos = position.xyz;



}