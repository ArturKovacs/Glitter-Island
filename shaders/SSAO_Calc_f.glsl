#version 330

uniform sampler2D objectNormal;
uniform sampler2D objectViewDepth;
uniform sampler2D objectDepth;
uniform sampler2D noiseTex;

uniform mat4 proj;
uniform mat4 projInv;
uniform mat4 viewTrInv;

uniform int screenWidth;
uniform int screenHeight;

const int numSamples = 48;
uniform vec3 sampleVecs[numSamples];

const float PI = 3.1415926535;

float calculateAOFactor(vec3 posView, vec3 normalView, vec3 randomVec)
{
	float occlusionValue = 0;
	
	vec3 tangent = normalize(randomVec - normalView * dot(randomVec, normalView));
	vec3 bitangent = cross(tangent, normalView);
	mat3 NTB = mat3(normalView, tangent, bitangent);
	
	for (int i = 0; i < numSamples; i += 1) {
		vec3 samplePosView = NTB*sampleVecs[i] + posView;
		
		vec4 samplePosScreen = proj * vec4(samplePosView, 1);
		samplePosScreen.xy /= samplePosScreen.w;
		samplePosScreen.xy = samplePosScreen.xy * vec2(0.5) + vec2(0.5);
		
		float visibleDepth = texture(objectViewDepth, samplePosScreen.xy).r;
		
		if (visibleDepth > samplePosView.z+0.01) {
			occlusionValue += smoothstep(0.0, 1.0, (samplePosView.z-visibleDepth)+1.5);
		}
	}
	
	return occlusionValue / numSamples;
}

void main()
{
	vec2 screenDimensions = vec2(screenWidth, screenHeight);
	float depth = texture(objectDepth, gl_FragCoord.xy/screenDimensions).x;
	
	vec3 normal = texture(objectNormal, gl_FragCoord.xy/screenDimensions).xyz;
	vec3 normalView = normalize((viewTrInv * vec4(normal, 1)).xyz);
	
	vec4 posView = projInv * vec4(vec3(gl_FragCoord.xy/screenDimensions, depth)*2 - vec3(1), 1);
	posView /= posView.w;
	
	float aoFactor = clamp(calculateAOFactor(posView.xyz, normalView.xyz, vec3(texture(noiseTex, posView.xy*64+posView.zx*128).xy, 0))*2, 0, 1);
	
	gl_FragColor = vec4(vec3(aoFactor), 1);
}
