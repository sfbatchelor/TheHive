#version 440 core

// these are for the programmable pipeline system and are passed in
// by default from OpenFrameworks
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelView;

in vec4 position;
in vec2 texcoord;

out vec2 vTexCoords;


void main()
{
    vTexCoords = texcoord;
	gl_Position = position;

}