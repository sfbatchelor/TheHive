#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords; //should be normalized

uniform sampler2D src;
uniform sampler2D depth;
uniform int DOF = 1;
  
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


uniform float focus = .01;
uniform float focusNear = 0.01;
uniform float focusFar = .04;

float LinearizeDepth(float zoverw)  
{  
	float n = focusNear; // camera z near  
	float f = focusFar; // camera z far  
	return (2.0 * n) / (f + n - zoverw * (f - n));  
}  

void main()
{             
    vec2 tex_offset = ( 1.0) / textureSize(src, 0); // gets size of single texel
    vec3 result = texture(src, TexCoords).rgb * weight[0]; // current fragment's contribution
	vec3 cul = vec3(0);


    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            cul += texture(src, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            cul += texture(src, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            cul += texture(src, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            cul += texture(src, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
	if(DOF == 1)
	{
		vec2 lookup = TexCoords;
		lookup.y *= -1;
		lookup.y += 1;
	    float depthVal = LinearizeDepth(texture(depth, lookup).r);      
		// DOF
//		float vfocus = clamp(focus ,0.,1.);
//		float dof = length(vfocus - depthVal);
//		depthVal = max(depthVal, 0.);
		result += (vec3(depthVal)*cul); 
	}
	else
		result += cul;

	FragColor = vec4(result, 1.0);

    //result = texture(src, TexCoords).rgb ; // current fragment's contribution
    //FragColor.rg =  (TexCoords)/2200;

}