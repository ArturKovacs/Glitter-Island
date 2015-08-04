#version 330

uniform sampler2D screen;
uniform sampler2D screenDepth;

uniform int screenWidth;
uniform int screenHeight;

uniform mat4 MVP;
//uniform mat4 invMVP;
flat in mat4 invViewProj;

uniform vec3 campos;
uniform float time;
uniform vec3 sunDir;

in vec3 posFromVert;
out vec4 fragColor;

float rand2(const in float seed){return cos(seed*14142.135623);}
//float rand(const in float seed){return (rand2(seed)+1)*0.5;} //1,4142135623730950488016887242097
float rand(const in float seed){return abs(rand2(seed));}

float smoothNoise(const in vec2 pos, const in vec2 seed)
{
	const int randomness = 20;
	
	const float freq = 50;
	const float amp = 2;
	
	float result = 0;
	for (int i = 0; i < randomness; i++){
		result += (pow((cos(pos.x*rand(i*3)*freq + seed.x*rand2(i*3+5))*cos(pos.y*rand(i*3+2)*freq + seed.y*rand2(i*3+6)))*0.5+0.5, 1)*2-1)*((rand(i*3+1)*0.9+0.1)*amp);
	}

	result *= 1.f/randomness;

	return result;
}

const vec3 waterColor = vec3(0.8, 0.88, 0.95);
const vec3 specColor = vec3(0.94, 0.94, 1);
const float shininess = 200;

void main(void)
{
	const float modulationStrength = 0.8;
	
	vec3 normal = normalize(vec3(0, 1, 0) + vec3(smoothNoise(posFromVert.xz+vec2(500), vec2(time*4)), smoothNoise(posFromVert.xz+vec2(100), vec2(time*6)), smoothNoise(posFromVert.xz+vec2(200), vec2(time*6)))*modulationStrength);
	vec3 viewDir = normalize(campos - posFromVert);
	vec3 halfway = normalize(sunDir + viewDir);
	
	float specularIntensity = pow(max(dot(normal, halfway), 0), shininess);
	
	vec2 fragCoordTex = vec2(float(gl_FragCoord.x)/screenWidth, float(gl_FragCoord.y)/screenHeight);
	
	vec4 screenSurfacePos = (invViewProj * vec4(
											(fragCoordTex.x*2-1),
											(fragCoordTex.y*2-1),
											(texelFetch(screenDepth, ivec2(gl_FragCoord.xy), 0).x)*2-1,
											1));
	
	screenSurfacePos = screenSurfacePos/screenSurfacePos.w;
	
	float screenDistInWater = distance(screenSurfacePos.xyz, posFromVert);
	float screenDistInWaterSqr;
	{
		vec3 posDiff = screenSurfacePos.xyz - posFromVert;
		screenDistInWaterSqr = dot(posDiff, posDiff);
	}
	
	vec3 fromCam = -viewDir;
	vec3 refractedDir = refract(fromCam, normal, 1/1.36);
	vec3 fakeHitPos = posFromVert+refractedDir*min(screenDistInWater, 1);
	//vec3 fakeHitPos = posFromVert+refractedDir;
	vec4 ClipSpacePos = MVP * vec4(fakeHitPos, 1);
	vec2 hitPosOnScreen = ((ClipSpacePos/ClipSpacePos.w).xy * 0.5) + vec2(0.5);
	float hitPosDepth = texture2D(screenDepth, hitPosOnScreen).x;
	
	vec3 refractedColor;
	float refractedDepth;
	if (hitPosDepth < gl_FragCoord.z-0.01) {
		hitPosOnScreen = fragCoordTex;
	}
	
	refractedColor = texture2D(screen, hitPosOnScreen).rgb;
	refractedDepth = texture2D(screenDepth, hitPosOnScreen).x;
	
	////////
	vec4 refractedSurfacePos = (invViewProj * vec4(
											(fragCoordTex.x*2-1),
											(fragCoordTex.y*2-1),
											(refractedDepth)*2-1,
											1));
	
	refractedSurfacePos = refractedSurfacePos/refractedSurfacePos.w;
	
	float distanceInWater = distance(refractedSurfacePos.xyz, posFromVert);
	//TODO: this is not the actual water depth
	float waterDepth = max(posFromVert.y - refractedSurfacePos.y, 0);
	////////
	
	//vec3 reflectedDir = reflect(fromCam, normal);
	//fakeHitPos = posFromVert+reflectedDir;
	//ClipSpacePos = MVP * vec4(fakeHitPos, 1);
	//hitPosOnScreen = ((ClipSpacePos/ClipSpacePos.w).xy * 0.5) + vec2(0.5);
	//vec3 reflectedColor = texture2D(screen, hitPosOnScreen.xy).rgb;
	
	//distanceInWater = mix(0, distanceInWater, -(exp(-waterDepth)-1));
	distanceInWater = mix(0, distanceInWater, clamp(waterDepth, 0, 1)); 
	fragColor = vec4(pow(waterColor, vec3(clamp(distanceInWater*0.4, 0, 25)))*(refractedColor /*+ reflectedColor*/)+(specColor*specularIntensity), 1);
}
