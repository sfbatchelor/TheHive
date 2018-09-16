#version 440 core
#extension GL_EXT_geometry_shader4 : enable

uniform float thickness;
uniform vec3 lightDir;
uniform mat4 modelViewProjectionMatrix;
uniform float minDist = 0.;
uniform float maxDist = 6000.;
uniform mat4 modelView;
uniform float radius = 1.;
uniform int uDivO = 7;
uniform int uDivA = 8;

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
	float sRadius = 1.;

    //Depende del polo en el que empezemos
    for (i= 0 ; i<= uDivO; i++){
        for (j = 0; j<=uDivA; j++) {


            if (i % 2 == 0){

            pz = cos (M_PI-(incA*j))*sRadius;
            py = sin (M_PI-(incA*j))*sin (incO*i)*sRadius;
            px = sin (M_PI-(incA*j))*cos (incO*i)*sRadius;

			// change colour based on distance away from camera (view)
			float dist = length(modelView* (gl_in[0].gl_Position + vec4(px, py, pz, 0.)));
			gl_Position =  modelViewProjectionMatrix *(gl_in[0].gl_Position + vec4(px, py, pz, 0.)); 
			float s = max(0,(dist - minDist))/(maxDist - minDist );
			fColour.rgb = mix(vColour[0].rgb, vColour[0].rgb*.1,s);
			EmitVertex();

			pz = cos (M_PI-(incA*j))*sRadius;
            py = sin (M_PI-(incA*j))*sin (incO*(i+1))*sRadius;
            px = sin (M_PI-(incA*j))*cos (incO*(i+1))*sRadius;
			dist = length(modelView* (gl_in[0].gl_Position + vec4(px, py, pz, 0.)));
			gl_Position =  modelViewProjectionMatrix * (gl_in[0].gl_Position + vec4(px, py, pz, 0.));
			s = max(0,(dist - minDist))/(maxDist - minDist );
			fColour.rgb = mix(vColour[0].rgb, vColour[0].rgb*.1,s);
			EmitVertex();

           }

        else {
            pz = cos (incA*j)*sRadius;
            py = sin (M_PI-(incA*j))*sin (incO*i)*sRadius;
            px = sin (M_PI-(incA*j))*cos (incO*i)*sRadius;

			float dist = length(modelView* (gl_in[0].gl_Position + vec4(px, py, pz, 0.)));
			gl_Position =  modelViewProjectionMatrix *(gl_in[0].gl_Position + vec4(px, py, pz, 0.)); 
			float s = max(0,(dist - minDist))/(maxDist - minDist );
			fColour.rgb = mix(vColour[0].rgb, vColour[0].rgb*.1,s);
			EmitVertex();

            pz = cos (incA*j)*sRadius;
            py = sin (incA*j)*sin (incO*(i+1))*sRadius;
            px = sin (incA*j)*cos (incO*(i+1))*sRadius;
			dist = length(modelView* (gl_in[0].gl_Position + vec4(px, py, pz, 0.)));
			gl_Position =  modelViewProjectionMatrix *(gl_in[0].gl_Position + vec4(px, py, pz, 0.)); 
			s = max(0,(dist - minDist))/(maxDist - minDist );
			fColour.rgb = mix(vColour[0].rgb, vColour[0].rgb*.1,s);
			EmitVertex();

       }
	   }
	   }


    EndPrimitive();

}