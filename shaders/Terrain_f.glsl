#version 330

#define LIGHT_CASCADE_COUNT 5

uniform sampler2DShadow cascadeShadowMaps[LIGHT_CASCADE_COUNT];
//uniform sampler2D cascadeShadowMaps[LIGHT_CASCADE_COUNT];
uniform mat4 worldToShadowMap[LIGHT_CASCADE_COUNT];
uniform float viewSubfrustumFarPlanesTexDepth[LIGHT_CASCADE_COUNT];

uniform sampler2D materialTexture;
uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform vec3 sunDir;
uniform vec3 sunColor;

in vec3 normal_v;
in vec2 texCoord_v;
in vec3 worldPos_v;

//out vec4 fragColor;

/// Stratified poisson sampling is taken from:
/// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/#Stratified_Poisson_Sampling
vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float checkerTex(const in vec2 pos)
{
	const float freq = 2;
	
	const float size = 1;
	const float edge = size*0.5;
	
	//return max(float((mod(pos.x*freq, size) > edge) ^^ (mod(pos.y*freq, size) > edge)), 0.3);
	return 1;
}

float calculateShadowFactor(
const in mat4 worldToShadowMap_curr,
const in sampler2DShadow cascadeShadowMaps_curr,
const in float bias)
{
	vec4 shadowMapNDCPos = worldToShadowMap_curr * vec4(worldPos_v, 1);
	shadowMapNDCPos = shadowMapNDCPos / shadowMapNDCPos.w;
	vec3 shadowMapTexPos = shadowMapNDCPos.xyz*0.5 + vec3(0.5);
	
	/*
	//const float bias = 0.001;
	float result = 0;
	// Sample the shadow map 4 times
	for (int i=0;i<4;i++) {
		// use either :
		//  - Always the same samples.
		//    Gives a fixed pattern in the shadow, but no noise
		//int index = i;
		//  - A random sample, based on the pixel's screen location. 
		//    No banding, but the shadow moves with the camera, which looks weird.
		int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
		//  - A random sample, based on the pixel's position in world space.
		//    The position is rounded to the millimeter to avoid too much aliasing
		//int index = int(16.0*random(floor(worldPos_v.xyz*100.0), i))%16;
		
		result += 0.25*texture(cascadeShadowMaps_curr, vec3(shadowMapTexPos.xy + poissonDisk[index]*0.0006, shadowMapTexPos.z-bias));
	}
	*/
	
	return texture(cascadeShadowMaps_curr, vec3(shadowMapTexPos.xy, shadowMapTexPos.z-bias));
	//float shadowMapDepth = texture(cascadeShadowMaps_curr, shadowMapTexPos.xy).x;
	//return 1-int(shadowMapDepth < (shadowMapTexPos.z)-0.0005);
	
	//return result;
}

const vec3 sandColor = vec3(0.98, 0.98, 0.96);

void main(void)
{
	vec2 texPos = texCoord_v*120;
	vec3 normal = normalize(normal_v);
	vec3 materialValue = texture(materialTexture, texCoord_v).rgb;
	
	vec3 sandTexSample = sandColor * texture(sandTexture, texPos).xyz * materialValue.b;
	vec3 grassTexSample = texture(grassTexture, texPos).xyz * materialValue.g;
	
	float flatSandValue = max(1-(materialValue.r+materialValue.g+materialValue.b), 0);
	vec3 flatSandSample = sandColor * flatSandValue;
	
	const vec3 ambientColor = vec3(0.04);
	vec3 diffuseColor = sunColor * max(dot(sunDir, normal), 0);
	
	//Select cascade
	int selectedCascade;
	for (int i = LIGHT_CASCADE_COUNT-1; i >= 0; i--) {
		selectedCascade = int(mix(selectedCascade, i, int(gl_FragCoord.z < viewSubfrustumFarPlanesTexDepth[i])));
	}
	
	float shadowFactor;
	
	switch (selectedCascade){
	case 0:
		shadowFactor = calculateShadowFactor(worldToShadowMap[0], cascadeShadowMaps[0], 0.0005);
		//diffuseColor *= vec3(0.3, .9, 0.3);
		break;
	case 1:
		shadowFactor = calculateShadowFactor(worldToShadowMap[1], cascadeShadowMaps[1], 0.0005);
		//diffuseColor *= vec3(0.3, 0.4, .9);
		break;
	case 2:
		shadowFactor = calculateShadowFactor(worldToShadowMap[2], cascadeShadowMaps[2], 0.0008);
		//diffuseColor *= vec3(.9, .9, 0.3);
		break;
	case 3:
		shadowFactor = calculateShadowFactor(worldToShadowMap[3], cascadeShadowMaps[3], 0.0015);
		//diffuseColor *= vec3(.9, 0.3, 0.3);
		break;
	case 4:
		shadowFactor = calculateShadowFactor(worldToShadowMap[4], cascadeShadowMaps[4], 0.0015);
		//diffuseColor *= vec3(.9, 0.3, 0.3);
		break;
	
	#if (LIGHT_CASCADE_COUNT != 5)
	ERROR_switch_case_count_does_not_match_light_cascade_count;
	#endif
	}
	
	diffuseColor *= shadowFactor;
	
	gl_FragData[0] = vec4((diffuseColor + ambientColor) * (flatSandSample + sandTexSample + grassTexSample) * checkerTex(texPos), 1.0);
	gl_FragData[1] = vec4(normal, 1);
}
