#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform sampler2D depth;
uniform float exposure = 1.9;

void main()
{             
    const float gamma = 1.3;
	vec2 bloomTexCoords = TexCoords; //not sure why gaussian blur highlight layer is upside down but fixed here
	bloomTexCoords.y *= -1.;
	bloomTexCoords.y += 1.;
	vec3 bloomColor= texture(bloomBlur, bloomTexCoords).rgb;
    vec3 hdrColor = texture(scene, (TexCoords)).rgb;       

	// BLOOM
    hdrColor += bloomColor; // additive blending
	vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // tone mapping
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}  