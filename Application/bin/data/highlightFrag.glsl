#version 440 core

in vec2 vTexCoords;

layout ( location = 0) out vec4 highlightColour;
uniform sampler2D src;

uniform float uBloomThreshhold = .9;

void main() {

	vec4 col = texture(src, vTexCoords);
	float brightness = length(col.rgb);

    // check whether fragment output is higher than threshold, if so output as brightness color
    if(brightness > uBloomThreshhold)
        highlightColour = vec4(vec3(brightness), 1.0);
    else
        highlightColour = vec4(0.0, 0.0, 0.0, 1.0);


}