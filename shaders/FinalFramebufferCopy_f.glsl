#version 330

uniform sampler2D colorTex;
uniform sampler2D depthTex;

in vec2 textureCoords;

out vec4 fragColor;

void main()
{
	//correct gamma!
	fragColor = pow(texelFetch(colorTex, ivec2(textureCoords), 0), vec4(1/2.2));
	//fragColor = vec4(vec3(texelFetch(depthTex, ivec2(textureCoords), 0).x), 1) + 0*texelFetch(colorTex, ivec2(textureCoords), 0);
	gl_FragDepth = texelFetch(depthTex, ivec2(textureCoords), 0).x;
}
