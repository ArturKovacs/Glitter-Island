#version 330

uniform sampler2D objectColor;
uniform sampler2D objectNormal;
uniform sampler2D objectDepth;

//uniform mat4 view_InvTr;
//uniform mat4 projInv;
//uniform mat4 proj;

uniform mat4 viewProj;
uniform mat4 viewProjInv;

uniform int screenWidth;
uniform int screenHeight;

const float PI = 3.1415926535;

float rand(vec2 seed)
{
	//return abs(fract(cos(seed.x*100)*seed.y + seed.x*cos(seed.y*321)*123));
	return fract(cos(seed.x)*seed.y);
}

float calculateAOFactor(vec3 posWorld, vec3 normalWorld, vec2 seed, float radius)
{
	float occludedSampleCount = 0;
	float validSampleCount = 0;
	
	const int numSamples = 50;
	for (int i = 0; i < numSamples; i += 1) {
		float theta = rand(seed + vec2(i*3))*PI;
		float phi = rand(seed + vec2(i*3+1))*2*PI;
		vec3 sampleDir = vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
		
		if (dot(normalWorld, sampleDir) > 0.02) {
			validSampleCount += 1.0;
			float radiusFactor = max(rand(seed + vec2(i*3+2)), 0.25);
			radiusFactor *= radiusFactor;
			vec3 samplePos = posWorld + sampleDir * radius * radiusFactor;
			vec4 samplePosScreen = viewProj * vec4(samplePos, 1);
			samplePosScreen /= samplePosScreen.w;
			samplePosScreen = (samplePosScreen+vec4(1))*0.5;
			float visibleDepth = texture(objectDepth, samplePosScreen.xy).x;
			//return visibleDepth;
			//occludedSampleCount = mix(occludedSampleCount, occludedSampleCount+1, float(visibleDepth < samplePosScreen.z-0.001));
			if (visibleDepth < samplePosScreen.z-0.001) {
				occludedSampleCount += 1;
			}
		}
	}
	
	if (validSampleCount == 0) {
		return 0.0;
	}
	
	return occludedSampleCount / validSampleCount;
}

void main()
{
	float depth = texelFetch(objectDepth, ivec2(gl_FragCoord.xy), 0).x;
	gl_FragDepth = depth;
	
	vec3 normal = texelFetch(objectNormal, ivec2(gl_FragCoord.xy), 0).xyz;
	//vec4 normalViewSpace = view_InvTr * vec4(normal, 0);
	//normalViewSpace /= normalViewSpace.w;
	//normalViewSpace.xyz = normalize(normalViewSpace.xyz);
	
	vec4 posWorld = viewProjInv * vec4(vec3(gl_FragCoord.xy/vec2(screenWidth, screenHeight), depth)*2 - vec3(1), 1);
	posWorld /= posWorld.w;
	
	float aoFactor = clamp(calculateAOFactor(posWorld.xyz, normal.xyz, gl_FragCoord.xy, 0.15)*2, 0, 1);
	
	gl_FragColor = vec4(texelFetch(objectColor, ivec2(gl_FragCoord.xy), 0).xyz * (1-aoFactor), 1);// *0 + vec4(normal.xyz*(1-1*aoFactor), 1);
	//gl_FragColor = vec4(texelFetch(objectColor, ivec2(gl_FragCoord.xy), 0).xyz * (1-aoFactor), 1) *0 + vec4(normal.xyz*(1-1*aoFactor), 1);
	//gl_FragColor = vec4(texelFetch(objectColor, ivec2(gl_FragCoord.xy), 0).xyz + vec3(0)*aoFactor, 1);
}
