#version 330

uniform sampler2D screen;
uniform sampler2D screenDepth;
uniform samplerCube skybox;

uniform int screenWidth;
uniform int screenHeight;

uniform mat4 MVP;
uniform mat4 invViewProj;

uniform vec3 campos;
uniform float time;
uniform vec3 sunDir;
uniform vec3 sunColor;

uniform sampler2D terrainHeightMap;
uniform vec2 terrainSizeXY;
uniform float terrainHeightScale;
uniform vec2 terrainPosXZ; //position of the (0, 0) texture coordinate of terrain heightmap in world space
uniform float waterHeight;

in vec3 posFromVert;
out vec4 fragColor;

float GetWaterDepth(vec2 posXZ)
{
	vec2 texPos = (posXZ-terrainPosXZ)/terrainSizeXY;
	texPos.y = -texPos.y;
	float terrainHeight = texture(terrainHeightMap, texPos).r * terrainHeightScale;
	
	return max(waterHeight - terrainHeight, 0);
}

const int waveCount = 16;

//wave directions
vec2 D[waveCount] = vec2[waveCount](
	vec2(-0.1659, 0.01467), vec2(0.501922, 0.52597), vec2(0.33277,-0.741945), vec2(-0.00580, -0.3592166),
	vec2(-0.3790,-0.92000), vec2(0.580932,-0.00818), vec2(0.18854,-0.422864), vec2(-0.32435, -0.9239267),
	vec2(-0.4968, 0.14807), vec2(0.196723, 0.44489), vec2(0.26122, 0.888485), vec2(0.963021, -0.5696051),
	vec2( 0.0422, 0.74602), vec2(0.487431, 0.10474), vec2(-0.5004, 0.990681), vec2(-0.72560, -0.0265197)
);

//wave frequencies
float w[waveCount] = float[waveCount](
	float(2.3), float(2.8), float(2.5), float(3.2),
	float(5.2), float(4.7), float(4.3), float(5.8),
	float(4.1), float(5.6), float(4.8), float(4.8),
	float(4.6), float(5.5), float(4.7), float(5.7)
);

//wave (frequencies*speeds) packed in phi
float phi[waveCount] = float[waveCount](
	w[ 0]*0.31, w[ 1]*0.52, w[ 2]*0.52, w[ 3]*0.52,
	w[ 4]*0.24, w[ 5]*0.34, w[ 6]*0.64, w[ 7]*0.64,
	w[ 8]*0.36, w[ 9]*0.46, w[10]*0.56, w[11]*0.46,
	w[12]*0.28, w[13]*0.23, w[14]*0.48, w[15]*0.30
);

//amplitudes
float A[waveCount] = float[waveCount](
	float(.008), float(.007), float(.007), float(.006),
	float(.005), float(.005), float(.005), float(.004),
	float(.004), float(.004), float(.005), float(.005),
	float(.004), float(.005), float(.005), float(.004)
);

vec3 GetWaveNormal(vec2 xy, float t)
{
	vec2 sum_d = vec2(0);
	
	const float k = 6;
	for (int i = 0; i < waveCount; i++) {
		//vec2 D_i = normalize(xy-D[i]*1000);
		vec2 D_i = normalize(D[i]);
		sum_d += A[i]*pow(sin(dot(D_i, xy)*w[i]+t*phi[i])*0.5+0.5, k-1)*
		             k*cos(dot(D_i, xy)*w[i]+t*phi[i])*
		             w[i]*D_i;
	}
	
	
	return normalize(vec3(-sum_d.x, -sum_d.y, 1.0));
}

const vec3 waterColor = vec3(0.8, 0.88, 0.95);
const float shininess = 512;
const float n = 1.335;
const float k = 0;
const float F0 = ((n-1)*(n-1)+k*k)/((n+1)*(n+1)+k*k);

void main(void)
{
	const float modulationStrength = 0.8;
	
	vec3 normal = GetWaveNormal(posFromVert.xz, time).xzy;
	vec3 viewDir = normalize(campos - posFromVert);
	vec3 halfway = normalize(sunDir + viewDir);
	
	float specularIntensity = pow(max(dot(normal, halfway), 0), shininess);
	
	vec2 screenDimensions = vec2(screenWidth, screenHeight);
	vec2 fragCoordTex = vec2(gl_FragCoord.xy/screenDimensions);
	
	vec4 underwaterSurfacePos = (invViewProj * vec4(
	                              (fragCoordTex.x*2-1),
	                              (fragCoordTex.y*2-1),
	                              (texelFetch(screenDepth, ivec2(gl_FragCoord.xy), 0).x)*2-1,
	                              1));
	
	underwaterSurfacePos.xyz /= underwaterSurfacePos.w;
	
	//float distanceInWater = distance(underwaterSurfacePos.xyz, posFromVert);
	
	float waterDepth = GetWaterDepth(posFromVert.xz);
	
	vec3 fromCam = -viewDir;
	vec3 refractedDir = refract(fromCam, normal, 1/n);
	vec3 fakeHitPos = posFromVert+refractedDir*waterDepth*0.75;
	vec4 ClipSpacePos = MVP * vec4(fakeHitPos, 1);
	vec2 hitPosOnScreen = (ClipSpacePos.xy/ClipSpacePos.w) * 0.5 + vec2(0.5);
	
	vec2 txy = vec2(1)-max(abs(fragCoordTex-0.5)-0.37, 0)*10.0;
	float refractionStrength = clamp(txy.x*txy.y, 0, 1);
	hitPosOnScreen = mix(fragCoordTex, hitPosOnScreen, refractionStrength);
	
	float hitPosDepth = texelFetch(screenDepth, ivec2(hitPosOnScreen*screenDimensions), 0).x;
	
	vec3 refractedColor;
	float refractedDepth;
	
	bool useFallbackSample = hitPosDepth <= gl_FragCoord.z;
	if (useFallbackSample) {
		hitPosOnScreen = fragCoordTex;
	}
	
	refractedColor = texture(screen, hitPosOnScreen).rgb;
	refractedDepth = texelFetch(screenDepth, ivec2(hitPosOnScreen*screenDimensions), 0).x;
	
	////////
	vec4 refractedSurfacePos = (invViewProj * vec4(
	                              (hitPosOnScreen.x*2-1),
	                              (hitPosOnScreen.y*2-1),
	                              (refractedDepth)*2-1,
	                              1));
	
	refractedSurfacePos.xyz /= refractedSurfacePos.w;
	////////
	
	vec3 reflectedDir = reflect(fromCam, normalize(vec3(0, 2, 0)+normal));
	//fakeHitPos = posFromVert+reflectedDir;
	//ClipSpacePos = MVP * vec4(fakeHitPos, 1);
	//hitPosOnScreen = ((ClipSpacePos/ClipSpacePos.w).xy * 0.5) + vec2(0.5);
	vec3 reflectedColor = texture(skybox, reflectedDir).rgb;
	
	float fresnel = clamp(F0 + (1-F0)*pow(1-dot(vec3(0, 1, 0), viewDir), 5), 0, 1);
	
	refractedColor *= 1-fresnel;
	reflectedColor *= fresnel;
	
	float distanceInWater = distance(refractedSurfacePos.xyz, posFromVert);
	
	distanceInWater = distanceInWater*clamp(distanceInWater, 0, 1); 
	//float foamStrength = pow(clamp(1-waterDepth, 0, 1), 1.5);
	float foamStrength = 0;
	fragColor = mix(
	vec4(pow(waterColor, vec3(clamp(distanceInWater*0.6, 0, 25)))*refractedColor + reflectedColor + sunColor*specularIntensity, 1),
	vec4(1),
	foamStrength);
}
