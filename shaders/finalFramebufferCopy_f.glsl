#version 330

uniform sampler2D colorTex;
uniform sampler2D depthTex;

in vec2 textureCoords;

out vec4 fragColor;

void main(void)
{
	//correct gamma!
	fragColor = pow(texelFetch(colorTex, ivec2(textureCoords), 0), vec4(1/2.2));
	//fragColor = texelFetch(colorTex, ivec2(textureCoords), 0);
	gl_FragDepth = texelFetch(depthTex, ivec2(textureCoords), 0).x;
}
