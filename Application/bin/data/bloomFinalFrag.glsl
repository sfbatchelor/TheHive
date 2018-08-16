#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;

layout ( location = 1) uniform sampler2D scene;
layout ( location = 0) uniform sampler2D bloomBlur;
uniform float exposure = 9.3;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, TexCoords/2).rgb;      
    vec3 bloomColor = vec3(0);//texture(bloomBlur, TexCoords*10).r;
    bloomColor.rgb = texture(bloomBlur, TexCoords*3).rgb;
    hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(hdrColor, 1.0);
}  