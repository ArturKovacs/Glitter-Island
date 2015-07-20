#version 330

uniform sampler2D materialTexture;
uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform vec3 lightDir;

in vec3 normalFromVert;
in vec2 texCoordFromVert;

out vec4 fragColor;

float rand2(const in float seed){return cos(seed*141421.35623);}
float rand(const in float seed){return abs(rand2(seed));}

float smoothNoise(const in vec2 pos)
{
	const int randomness = 7;
	
	const float freq = 25000;
	
	float result = 0;
	for (int i = 0; i < randomness; i++){
		result += clamp((((cos(pos.x*rand(i*3)*freq)*cos(pos.y*rand(i*3+2)*freq))*(clamp(rand(i*3+1)*50, 0.5, 1)))*4)+4, 0, 1);
	}

	result *= 1.f/randomness;

	return result;
	//return (result*0.5)+0.5;
}


float sandGrainRand(const in vec2 pos)
{
	//return pow(rand(rand(pos.x)+rand(pos.y)), 150)*0.5+0.5;
	return clamp(pow(smoothNoise(pos), 3), 0.6, 1);
}

float checkerTex(const in vec2 pos)
{
	const float freq = 2;
	
	const float size = 1;
	const float edge = size*0.5;
	
	return max(float((mod(pos.x*freq, size) > edge) ^^ (mod(pos.y*freq, size) > edge)), 0.3);
}

const vec3 sandColor = vec3(0.98, 0.98, 0.96);

void main(void) 
{ 
	vec2 texPos = texCoordFromVert*60;
	vec3 normal = normalize(normalFromVert);
	vec3 materialValue = texture(materialTexture, texCoordFromVert).rgb;
	
	vec3 sandSample = sandColor * texture(sandTexture, texPos).xyz * materialValue.b;
	vec3 grassSample = texture(grassTexture, texPos).xyz * materialValue.g;
	
	float defaultValue = max(1-(materialValue.x+materialValue.y+materialValue.z), 0);
	
	fragColor = vec4(max(dot(lightDir, normal), 0) * (sandSample + grassSample + defaultValue*sandColor) * checkerTex(texPos), 1.0); 
} 