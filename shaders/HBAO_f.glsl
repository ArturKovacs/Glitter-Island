#version 330

uniform sampler2D objectColor;
uniform sampler2D objectNormal;
uniform sampler2D objectDepth;

void main()
{
	float depth = texelFetch(objectDepth, ivec2(gl_FragCoord.xy), 0).x;
	gl_FragDepth = depth;
	//gl_FragDepth = 0;
	gl_FragColor = vec4(texelFetch(objectColor, ivec2(gl_FragCoord.xy), 0).xyz, 1);
	//gl_FragColor = vec4(texelFetch(objectNormal, ivec2(gl_FragCoord.xy), 0).xyz, 1);
}
