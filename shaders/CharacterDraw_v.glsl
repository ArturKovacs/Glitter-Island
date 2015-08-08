#version 330

uniform mat4 MVP;
uniform vec2 texCoordMin;
uniform vec2 texCoordMax;

in vec2 vertexPos;
//in vec2 vertexTexCoord;

out vec2 texCoord_v;

void main(void)
{
	texCoord_v = texCoordMin + (texCoordMax-texCoordMin)*vertexPos;
	gl_Position = MVP * vec4(vertexPos, 0.0, 1.0);
}
