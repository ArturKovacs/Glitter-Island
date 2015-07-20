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
	const int randomness = 7;
	
	const float freq = 15;
	
	float result = 0;
	for (int i = 0; i < randomness; i++){
		result += (pow((cos(pos.x*rand(i*3)*freq + seed.x*rand2(i*3+5))*cos(pos.y*rand(i*3+2)*freq + seed.y*rand2(i*3+6)))*0.5+0.5, 1)*2-1)*(rand(i*3+1)*0.9+0.1);
	}

	result *= 1.f/randomness;

	return result;
}

const vec3 waterColor = vec3(0.8, 0.88, 0.95);
const vec3 specColor = vec3(0.94, 0.94, 1);
const float shininess = 200;

void main(void)
{
	float objectsDepth = texelFetch(screenDepth, ivec2(gl_FragCoord.xy), 0).x;
	
	const float modulationStrength = 0.8;
	
	vec3 normal = normalize(vec3(0, 1, 0) + vec3(smoothNoise(posFromVert.xz+vec2(500), vec2(time*4)), smoothNoise(posFromVert.xz+vec2(100), vec2(time*6)), smoothNoise(posFromVert.xz+vec2(200), vec2(time*6)))*modulationStrength);
	vec3 viewDir = normalize(campos - posFromVert);
	vec3 halfway = normalize(sunDir + viewDir);
	
	float specularIntensity = pow(max(dot(normal, halfway), 0), shininess);
	
	////////
	
	vec4 terrainPointPos = (invViewProj * vec4(((float(gl_FragCoord.x)/screenWidth)*2-1),
											((float(gl_FragCoord.y)/screenHeight)*2-1),
											(objectsDepth)*2-1,
											1));
	
	terrainPointPos = terrainPointPos/terrainPointPos.w;
	
	float terrainDistance = distance(terrainPointPos.xyz, posFromVert);
	//TODO: this is not the actual water depth
	float waterDepth = abs(terrainPointPos.y - posFromVert.y);
	
	////////
	
	vec3 fromCam = normalize(posFromVert - campos);
	vec3 refractedDir = refract(fromCam, normal, 1/1.36);
	vec3 fakeHitPos = posFromVert+refractedDir*min(terrainDistance, 1);
	vec4 ClipSpacePos = MVP * vec4(fakeHitPos, 1);
	vec2 hitPosOnScreen = ((ClipSpacePos/ClipSpacePos.w).xy * 0.5) + vec2(0.5);
	float hitPosDepth = texture2D(screenDepth, hitPosOnScreen.xy).x;
	//vec3 refractedColor = texture2D(screen, hitPosOnScreen.xy).rgb;
	//vec3 refractedColor = texelFetch(screen, ivec2(gl_FragCoord.xy), 0).rgb;
	//if (hitPosDepth < gl_FragCoord.z) {
	//	refractedColor *= 0.5;
	//}
	
	vec3 refractedColor;
	if (hitPosDepth > gl_FragCoord.z+0.0005) {
		refractedColor = texture2D(screen, hitPosOnScreen.xy).rgb;
	}
	else {
		refractedColor = texelFetch(screen, ivec2(gl_FragCoord.xy), 0).rgb;
	}
	
	vec3 reflectedDir = reflect(fromCam, normal);
	fakeHitPos = posFromVert+reflectedDir;
	ClipSpacePos = MVP * vec4(fakeHitPos, 1);
	hitPosOnScreen = ((ClipSpacePos/ClipSpacePos.w).xy * 0.5) + vec2(0.5);
	vec3 reflectedColor = texture2D(screen, hitPosOnScreen.xy).rgb;
	
	//const float maxDist = 4.5;
	//float transformedDist = (-1/(terrainDistance+(1/maxDist)))+maxDist;
	fragColor = vec4(pow(waterColor, vec3(clamp(terrainDistance*0.4, 0, 25)))*(refractedColor /*+ reflectedColor*/)+(specColor*specularIntensity), clamp(waterDepth*20, 0, 1));
}
