#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform sampler2D depth;
uniform float exposure = 1.9;
uniform float focus = .1;
uniform float focusNear = 0.1;
uniform float focusFar = 10.4;

float LinearizeDepth(float zoverw)  
{  
	float n = focusNear; // camera z near  
	float f = focusFar; // camera z far  
	return (2.0 * n) / (f + n - zoverw * (f - n));  
}  
void main()
{             
    const float gamma = 1.5;
	vec3 bloomColor= texture(bloomBlur, TexCoords/2.).rgb;
    vec3 hdrColor = texture(scene, TexCoords*2).rgb;      
    float depthVal = LinearizeDepth(texture(depth, TexCoords).r);      

	// BLOOM
    hdrColor += bloomColor; // additive blending
	//vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	// DOF
	float vfocus = clamp(focus ,0.,1.);
	float depthDiff = length(vfocus - depthVal);
	vec3 result = mix(hdrColor, bloomColor, 0);
	
    // tone mapping
    // also gamma correct while we're at it       
    //result = pow(result, vec3(1.0 / gamma));




    FragColor = vec4(hdrColor, 1.0);
    //FragColor = vec4(bloomColor, 1.0);
   //FragColor = vec4(depthDiff,0,0, 1.0);
}  