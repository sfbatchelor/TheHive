#version 440


struct Point{
	vec4 pos;
	vec4 col;
	vec4 vel;
};

const float EPSILON = 0.0001;

layout(std140, binding=0) buffer newPoints{
    Point np[];
};
layout(std140, binding=1) buffer prevPoints{
    Point pp[];
};

layout(rgba8, binding=0) uniform readonly image2D src;

uniform float uTime = 1.0;
uniform int uNumPointsSF = 1;



uniform float uWidth = 1000.;
uniform float uHeight = 1000.;
uniform float uDepth = 10000.;
uniform float uAccelScale = .005;

uniform float uMinDepth = -300.;
uniform float uMaxDepth = 300.;

uniform int uNumFftBands = 512;
uniform float uFft [512];


uniform vec2 uPixSampleSize = vec2(10.);// 100 sample pixels
uniform float uAlignPointSampleRadius = 5; // ~10 pointrs - very close 
uniform float uSeperationPointSampleRadius = 20; // ~100 points - surrounding area

uniform float uMaxCohDist = 100;
uniform float uMinCohDist = 0;
float uCohDistRange = uMaxCohDist - uMinCohDist;
uniform float uMaxCohAccel = .00005;


uniform float uNearAlignDist = 1.;
uniform float uFarAlignDist = 10.;
uniform float uAlignSpeedDiffMax= 5.;
uniform float uAlignSpeedMag = 0.0003;

uniform float uSepFalloffDist = 10.;
uniform float uSepDistMag = .1;

// in 2d on the x-y plane, assuming a & b are normalized
// returns -1 if heading is left, +1 if right

int leftOrRight(vec3 a, vec3 b)
{
	vec3 cp = normalize(cross(a, b));
	if(cp.y < 0.0 )
		return 1;
	else 
		return -1;
}

vec3 safeNormalize(vec3 vec) 
{
	if(length(vec) == 0 ) 
		return vec3(0);
	return normalize(vec);
}



// 2D Random
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

vec3 rule1(vec3 my_pos, vec3 their_pos){
	vec3 dir = my_pos-their_pos;
	float sqd = dot(dir,dir);
	if(sqd < 300.0*300.0){
		return dir;
	}
	return vec3(0.0);
} 

vec3 rule2(vec3 my_pos, vec3 their_pos, vec3 my_vel, vec3 their_vel){
	vec3 d = their_pos - my_pos;
	vec3 dv = their_vel - my_vel;
	return dv / (dot(dv,dv) + 10.0);
}

vec3 rule3(vec3 my_pos, vec3 their_pos){
	vec3 dir = their_pos-my_pos;
	float sqd = dot(dir,dir);
	if(sqd < 50.0*50.0 && length(dir) != 0 ){
		float f = 1000000.0/sqd;
		return normalize(dir)*f;
	}
	return vec3(0.0);
}

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

void main(){

// SETUP
	// point and some precomputed info
	Point point = pp[gl_GlobalInvocationID.x];
	vec3 pDir = vec3(0);
	vec3 correctionRt = vec3(0);
	if(length(point.vel.xyz) != 0)
	{
		vec3 pDir = safeNormalize(point.vel.xyz);
		vec3 correctionRt = safeNormalize(cross(pDir, vec3(0,0,1)));
	}

	// values for particle sampling
	uint m = uint(1024.0*uNumPointsSF*uTime);
	uint start = uint(mod(m, 1024*uNumPointsSF-512));
	uint end = start + 512;

	//accel value to add to vel, vel to add to pos
	vec3 totalAccel = vec3(0);

	// Lookup Pixels from underlying image
	vec2 lookup = vec2(point.pos.xy);
	float xMin = lookup.x - (uPixSampleSize.x/2.);
	float xMax = lookup.x + (uPixSampleSize.x/2.);
	float yMin = lookup.y - (uPixSampleSize.y/2.);
	float yMax = lookup.y + (uPixSampleSize.y/2.);



// COLOUR COHESION
	// 1- find the closest matching pixel colour in an area, set that pixel pos to be the new 'target'
	float closestColL = 100000.;
	vec3 cohesionTarget = point.pos.xyz;
	for (float x = xMin; x <= xMax; x+= 1.)
	{
		for (float y = yMin; y <= yMax; y+= 1.)
		{

			vec2 tmpXY = vec2(x,y);
			if(x < lookup.x) //round up if needed - due to pixel int value lookups
				tmpXY.x = ceil(x);
			if(y < lookup.y)
				tmpXY.y = ceil(y);
			ivec2 imageXY = ivec2(tmpXY);
			vec4 imageCol = imageLoad(src, imageXY);
			float imageDiff = length(  point.col.rgb -imageCol.rgb  );

			float brightness = length(imageCol.rgb);
			brightness = clamp(brightness, 0 ,255);
			brightness /= 255.;
			float depth = ((uMaxDepth - uMinDepth) * brightness) + uMinDepth;
			cohesionTarget = vec3(x,y, 0);

			// 2- 'Target' - point pos, normalize for direction
			vec3 toTarget = cohesionTarget - point.pos.xyz;
			vec3 toTargetDir = safeNormalize(toTarget);
			// 3 - Get distance to centre (float), clamp to min and max values (0 to 2). 
			//     Calc accel strength = dist2Centre/distRange. So strength is faster the further away the point is
			float toTargetDist = length(toTarget);
			toTargetDist = clamp(toTargetDist, uMinCohDist, uMaxCohDist);
			float cohAccelMag =  (toTargetDist/ uCohDistRange) * uMaxCohAccel;

			float colSig = 1.0 - imageDiff; //force is more the more similar they are
			
			// 4 - Add dir*accel strength to accel;
			totalAccel += toTargetDir * cohAccelMag * colSig;
		}
	}




// POINT LOOKUP BEHAVIOURS: FUZZY-ALLIGNMENT PARTICLES & SEPERATION 
	vec3 fuzzySpeed = vec3(0);
	vec3 fuzzyHeading = vec3(0);
	vec3 seperation = vec3(0);
	for(uint i=start;i<end;i++){
		if(i!=gl_GlobalInvocationID.x){
			// significance based on distance away
			Point samplePoint = pp[i];
			totalAccel += rule1(point.pos.xyz, samplePoint.pos.xyz) * .007;
			totalAccel += rule2(point.pos.xyz,samplePoint.pos.xyz, samplePoint.vel.xyz, samplePoint.vel.xyz) * 50;
			totalAccel += rule3(point.pos.xyz,samplePoint.pos.xyz) *.0018 ;
		}
	}

	vec3 noiseDir = vec3((noise(point.pos.xy/1000) - .5)*2.);
	noiseDir.y = (noise(point.pos.xx/1000) - .5)*.2;
	noiseDir.z = (noise(point.pos.xz/1000) - .5)*.2;




// SOUND STUFF
//	vec4 imCol = imageLoad(src, ivec2(lookup));
//	float brightness =  length(imCol.rgb)/255.;
//	brightness = clamp(brightness, 0, 1);
//	int fftLookup = int(brightness * uNumFftBands);
//	fftLookup = clamp(fftLookup, 0, uNumFftBands - 1);
//	float soundVal = uFft[1];
//	totalAccel *= soundVal*30;

// INTEGRATION 
	// 1 - Velocity += Accel

	point.vel.xyz += totalAccel* uAccelScale;
	point.vel.xyz *= .99;
	// 2- Boundary check
	if( point.pos.y < -uHeight/2. || point.pos.y > uHeight/2. || point.pos.x < -uWidth/2. || point.pos.x > uWidth/2. || point.pos.z < -uDepth/2. || point.pos.z > uDepth/2.)
	{
		point.vel *= -1;

		if(point.pos.y < -uHeight/2.)
			point.pos.y = -uHeight/2.;
		if(point.pos.y > uHeight/2.)
			point.pos.y = uHeight/2.;
		if(point.pos.x < -uWidth/2.)
			point.pos.x = -uWidth/2.;
		if(point.pos.x > uWidth/2.)
			point.pos.x = uWidth/2.;

		if(point.pos.z > uDepth/2.)
			point.pos.z = uDepth/2.;
		if(point.pos.z < -uDepth/2.)
			point.pos.z = -uDepth/2.;
	}
	else 
		point.pos += point.vel;


// WRITE OUT
	np[gl_GlobalInvocationID.x ] = point;
}