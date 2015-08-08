#version 330

uniform sampler2D screenColor;
uniform sampler2D screenDepth;

out vec4 fragColor;

vec4 fogFunction(const in vec4 originalColor, const in vec4 fogColor, const in float depth)
{
	float weight = clamp(depth*pow(depth, pow(1024, depth)), 0, 1);
	return mix(originalColor, fogColor, weight);
}

void main()
{
	const vec4 fogColor = vec4(0.7, 0.8, 1, 1);
	float depth = texelFetch(screenDepth, ivec2(gl_FragCoord.xy), 0).x;
	fragColor = fogFunction(texelFetch(screenColor, ivec2(gl_FragCoord.xy), 0), fogColor, depth);
}
