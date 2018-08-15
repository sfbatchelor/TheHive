#version 440 core
#extension GL_EXT_geometry_shader4 : enable

uniform float thickness;
uniform vec3 lightDir;
uniform mat4 modelViewProjectionMatrix;
uniform float radius = 1.5;
uniform int uDivO = 8;
uniform int uDivA = 7;

layout (points) in;
layout (triangle_strip, max_vertices = 128) out;

in vec4 vColour[];
in float vSoundVal[];
out vec4 fColour;

#define M_PI 3.1415926535

void main() {


	float px, py,pz;
    int i,j;

    float incO = 2*M_PI / uDivO;
    float incA = M_PI /uDivA;
	//float sRadius = (radius * vSoundVal[0])+.6;
	float sRadius = 2.6;

    //Depende del polo en el que empezemos
    for (i= 0 ; i<= uDivO; i++){
        for (j = 0; j<=uDivA; j++) {


            if (i % 2 == 0){

            pz = cos (M_PI-(incA*j))*sRadius;
            py = sin (M_PI-(incA*j))*sin (incO*i)*sRadius;
            px = sin (M_PI-(incA*j))*cos (incO*i)*sRadius;

			gl_Position =  modelViewProjectionMatrix *(gl_in[0].gl_Position + vec4(px, py, pz, 0.)); 
			fColour = vColour[0];
			EmitVertex();

			pz = cos (M_PI-(incA*j))*sRadius;
            py = sin (M_PI-(incA*j))*sin (incO*(i+1))*sRadius;
            px = sin (M_PI-(incA*j))*cos (incO*(i+1))*sRadius;

			gl_Position =  modelViewProjectionMatrix *(gl_in[0].gl_Position + vec4(px, py, pz, 0.)); 
			fColour = vColour[0];
			EmitVertex();

           }

        else {
            pz = cos (incA*j)*sRadius;
            py = sin (M_PI-(incA*j))*sin (incO*i)*sRadius;
            px = sin (M_PI-(incA*j))*cos (incO*i)*sRadius;

			gl_Position =  modelViewProjectionMatrix *(gl_in[0].gl_Position + vec4(px, py, pz, 0.)); 
			fColour = vColour[0];
			EmitVertex();

            pz = cos (incA*j)*sRadius;
            py = sin (incA*j)*sin (incO*(i+1))*sRadius;
            px = sin (incA*j)*cos (incO*(i+1))*sRadius;

			gl_Position =  modelViewProjectionMatrix *(gl_in[0].gl_Position + vec4(px, py, pz, 0.)); 
			fColour = vColour[0];
			EmitVertex();

       }
	   }
	   }


    EndPrimitive();

}