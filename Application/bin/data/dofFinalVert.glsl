#version 440 core
in vec4 position;
in vec2 texcoord;

uniform mat4 modelViewProjectionMatrix;

out vec2 TexCoords;

void main()
{
    TexCoords = texcoord;
    gl_Position = ( position);
}

