#version 330

#define LIGHT_CASCADE_COUNT 5

uniform sampler2DShadow cascadeShadowMaps[LIGHT_CASCADE_COUNT];
uniform mat4 worldToShadowMap[LIGHT_CASCADE_COUNT];
uniform float viewSubfrustumFarPlanesTexDepth[LIGHT_CASCADE_COUNT];

uniform sampler2D sandTexture;
uniform sampler2D sandNormalMap;
uniform sampler2D grassTexture;
uniform sampler2D grassNormalMap;
uniform sampler2D materialTexture;
uniform vec3 sunDir;
uniform vec3 sunColor;

in vec3 normal_v;
in vec3 tangent_v;
in vec2 texCoord_v;
in vec3 worldPos_v;
in float viewZ_v;

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
	
	vec2 texSize = vec2(2048, 1024);
	float radius = 1;
	
	float accumValue = 0;
	
	for (float dx = -radius; dx <= radius + 0.01; dx += 1) {
		for (float dy = -radius; dy <= radius + 0.01; dy += 1) {
			accumValue += texture(cascadeShadowMaps_curr, vec3(shadowMapTexPos.xy + (vec2(dx, dy)/texSize), shadowMapTexPos.z-bias));
		}
	}
	
	float kernelSize = radius*2 + 1;
	kernelSize *= kernelSize;
	
	return accumValue / kernelSize;
	
	//return texture(cascadeShadowMaps_curr, vec3(shadowMapTexPos.xy, shadowMapTexPos.z-bias));

}

const vec3 sandColor = vec3(0.98, 0.98, 0.96);

void main(void)
{
	vec2 texPos = texCoord_v*120;
	vec3 normal = normalize(normal_v);
	vec3 tangent = normalize(tangent_v - normal * dot(normal, tangent_v));
	vec3 bitangent = cross(tangent, normal);
	mat3 textureToWorld = mat3(tangent, bitangent, normal);
	
	vec3 materialValue = texture(materialTexture, texCoord_v).rgb;
	
	vec3 sandTexSample = sandColor * texture(sandTexture, texPos).xyz * materialValue.b;
	vec3 sandNormalSample = textureToWorld * (texture(sandNormalMap, texPos).xyz*2 - vec3(1));
	sandNormalSample *= materialValue.b;
	
	vec3 grassTexSample = texture(grassTexture, texPos).xyz * materialValue.g;
	vec3 grassNormalSample = textureToWorld * (texture(grassNormalMap, texPos).xyz*2 - vec3(1));
	grassNormalSample *= materialValue.g;
	
	float flatSandValue = max(1-(materialValue.r+materialValue.g+materialValue.b), 0);
	vec3 flatSandSample = sandColor * flatSandValue;
	vec3 flatSandNormalSample = normal * flatSandValue;
	
	normal = normalize(flatSandNormalSample + grassNormalSample + sandNormalSample);
	
	const vec3 ambientColor = vec3(0.03);
	vec3 diffuseColor = sunColor * max(dot(sunDir, normal), 0);
	
	//Select cascade
	int selectedCascade;
	for (int i = LIGHT_CASCADE_COUNT-1; i >= 0; i--) {
		selectedCascade = int(mix(selectedCascade, i, int(gl_FragCoord.z < viewSubfrustumFarPlanesTexDepth[i])));
	}
	
	float shadowFactor;
	
	switch (selectedCascade){
	case 0:
		shadowFactor = calculateShadowFactor(worldToShadowMap[0], cascadeShadowMaps[0], 0.00);
		//diffuseColor *= vec3(0.3, .9, 0.3);
		break;
	case 1:
		shadowFactor = calculateShadowFactor(worldToShadowMap[1], cascadeShadowMaps[1], 0.00);
		//diffuseColor *= vec3(0.3, 0.4, .9);
		break;
	case 2:
		shadowFactor = calculateShadowFactor(worldToShadowMap[2], cascadeShadowMaps[2], 0.00);
		//diffuseColor *= vec3(.9, .9, 0.3);
		break;
	case 3:
		shadowFactor = calculateShadowFactor(worldToShadowMap[3], cascadeShadowMaps[3], 0.00);
		//diffuseColor *= vec3(.9, 0.3, 0.3);
		break;
	case 4:
		shadowFactor = calculateShadowFactor(worldToShadowMap[4], cascadeShadowMaps[4], 0.00);
		//diffuseColor *= vec3(.9, 0.3, 0.3);
		break;
	
	#if (LIGHT_CASCADE_COUNT != 5)
	ERROR_switch_case_count_does_not_match_light_cascade_count;
	#endif
	}
	
	diffuseColor *= shadowFactor;
	
	gl_FragData[0] = vec4((diffuseColor + ambientColor) * 
		(flatSandSample + sandTexSample + grassTexSample) * checkerTex(texPos), 1.0);
		
	gl_FragData[1] = vec4(normal, 1);
	gl_FragData[2] = vec4(vec3(viewZ_v), 1);
}
