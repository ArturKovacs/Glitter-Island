#version 330

uniform sampler2D texContainingAlpha;

in vec2 texCoord_v;

out vec4 fragColor;

void main(void) 
{ 
	//Depth only
	float depth = mix(1, gl_FragCoord.z, texture(texContainingAlpha, texCoord_v).a);
	gl_FragDepth = depth;
	fragColor = vec4(vec3(depth), 1);
}
