#version 330

uniform sampler2D screen;
uniform sampler2D screenDepth;

uniform int screenWidth;
uniform int screenHeight;

uniform mat4 MVP;
uniform mat4 invViewProj;

uniform vec3 campos;
uniform float time;
uniform vec3 sunDir;
uniform vec3 sunColor;

in vec3 posFromVert;
out vec4 fragColor;

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
	float(4.3), float(9.8), float(6.9), float(6.0),
	float(7.2), float(4.7), float(9.9), float(5.9),
	float(5.1), float(5.6), float(4.8), float(7.8),
	float(6.0), float(10.5), float(8.7), float(5.7)
);

//wave frequencies*speeds packed in phi
float phi[waveCount] = float[waveCount](
	w[ 0]*0.31, w[ 1]*0.52, w[ 2]*0.52, w[ 3]*0.52,
	w[ 4]*0.24, w[ 5]*0.34, w[ 6]*0.64, w[ 7]*0.64,
	w[ 8]*0.36, w[ 9]*0.46, w[10]*0.56, w[11]*0.46,
	w[12]*0.28, w[13]*0.23, w[14]*0.48, w[15]*0.30
);

//amplitudes
float A[waveCount] = float[waveCount](
	float(.005), float(.006), float(.004), float(.005),
	float(.003), float(.006), float(.003), float(.005),
	float(.006), float(.003), float(.006), float(.006),
	float(.005), float(.004), float(.005), float(.006)
);

vec3 getWaveNormal(vec2 xy, float t)
{
	vec2 sum_d = vec2(0);
	
	const float k = 3;
	for (int i = 0; i < waveCount; i++) {
		vec2 D_i = normalize(xy-D[i]*1000);
		sum_d += A[i]*pow(sin(dot(D_i, xy)*w[i]+t*phi[i])*0.5+0.5, k-1)*
		             k*cos(dot(D_i, xy)*w[i]+t*phi[i])*
		             w[i]*D_i;
	}
	
	
	return normalize(vec3(-sum_d.x, -sum_d.y, 1.0));
}

const vec3 waterColor = vec3(0.8, 0.88, 0.95);
const float shininess = 256;

void main(void)
{
	const float modulationStrength = 0.8;
	
	vec3 normal = getWaveNormal(posFromVert.xz, time).xzy;
	vec3 viewDir = normalize(campos - posFromVert);
	vec3 halfway = normalize(sunDir + viewDir);
	
	float specularIntensity = pow(max(dot(normal, halfway), 0), shininess);
	
	vec2 screenDimensions = vec2(screenWidth, screenHeight);
	vec2 fragCoordTex = vec2(gl_FragCoord.xy/screenDimensions);
	
	vec4 screenSurfacePos = (invViewProj * vec4(
	                              (fragCoordTex.x*2-1),
	                              (fragCoordTex.y*2-1),
	                              (texelFetch(screenDepth, ivec2(gl_FragCoord.xy), 0).x)*2-1,
	                              1));
	
	screenSurfacePos.xyz /= screenSurfacePos.w;
	
	float screenDistInWater = distance(screenSurfacePos.xyz, posFromVert);
	float screenDistInWaterSqr;
	{
		vec3 posDiff = screenSurfacePos.xyz - posFromVert;
		screenDistInWaterSqr = dot(posDiff, posDiff);
	}
	
	vec3 fromCam = -viewDir;
	vec3 refractedDir = refract(fromCam, normal, 1/1.36);
	vec3 fakeHitPos = posFromVert+refractedDir*min(screenDistInWater, 0.25);
	vec4 ClipSpacePos = MVP * vec4(fakeHitPos, 1);
	vec2 hitPosOnScreen = (ClipSpacePos.xy/ClipSpacePos.w) * 0.5 + vec2(0.5);
	float hitPosDepth = texelFetch(screenDepth, ivec2(hitPosOnScreen*screenDimensions), 0).x;
	
	vec3 refractedColor;
	float refractedDepth;
	if (hitPosDepth < gl_FragCoord.z) {
		hitPosOnScreen = fragCoordTex;
	}
	
	refractedColor = texture(screen, hitPosOnScreen).rgb;
	refractedDepth = texelFetch(screenDepth, ivec2(hitPosOnScreen*screenDimensions), 0).x;
	
	////////
	vec4 refractedSurfacePos = (invViewProj * vec4(
	                              (fragCoordTex.x*2-1),
	                              (fragCoordTex.y*2-1),
	                              (refractedDepth)*2-1,
	                              1));
	
	refractedSurfacePos.xyz /= refractedSurfacePos.w;
	
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
	fragColor = vec4(pow(waterColor, vec3(clamp(distanceInWater*0.4, 0, 25)))*(refractedColor /*+ reflectedColor*/)+(sunColor*specularIntensity), 1);
}
