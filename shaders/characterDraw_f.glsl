#version 330

uniform sampler2D fontTexture;
uniform vec4 characterColor;


in vec2 texCoord_v;

out vec4 fragColor;

void main(void) 
{ 
	fragColor = texture(fontTexture, texCoord_v) * characterColor;
} 
