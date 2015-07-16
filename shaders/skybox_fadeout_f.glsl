#version 330

uniform sampler2D screenColor;
uniform sampler2D screenDepth;
uniform sampler2D skyboxColor;

out vec4 fragColor;

vec4 fogFunction(const in vec4 originalColor, const in vec4 fogColor, const in float depth)
{
	float weight = clamp(depth*pow(depth, pow(1024, depth)), 0, 1);
	weight *= fogColor.w;
	return mix(originalColor, fogColor, weight);
}

void main()
{
	//const vec4 fogColor = vec4(0.7, 0.8, 1, 1);
	vec4 fogColor = texelFetch(skyboxColor, ivec2(gl_FragCoord.xy), 0);
	float depth = texelFetch(screenDepth, ivec2(gl_FragCoord.xy), 0).x;
	gl_FragDepth = depth;
	//gl_FragDepth = 0;
	fragColor = vec4(fogFunction(vec4(texelFetch(screenColor, ivec2(gl_FragCoord.xy), 0).xyz, 1), fogColor, depth).xyz, 1);
	//fragColor = vec4(fogFunction(vec4(texelFetch(screenColor, ivec2(gl_FragCoord.xy), 0).xyz, 1), fogColor, depth).xyz*0+vec3(0, 1, 0), 1);
}
