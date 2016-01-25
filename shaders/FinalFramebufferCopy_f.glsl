#version 330

uniform sampler2D colorTex;
uniform sampler2D depthTex;

uniform float exposure;

in vec2 textureCoords;

out vec4 fragColor;

void main()
{
	//tone mapping
	vec4 color = vec4(1) - exp2(-texelFetch(colorTex, ivec2(textureCoords), 0) * exposure);

	//correct gamma!
	fragColor = pow(color, vec4(1/2.2));
	
	//fragColor = vec4(vec3(texelFetch(depthTex, ivec2(textureCoords), 0).x), 1) + 0*texelFetch(colorTex, ivec2(textureCoords), 0);
	gl_FragDepth = texelFetch(depthTex, ivec2(textureCoords), 0).x;
}
