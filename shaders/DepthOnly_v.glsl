#version 330

uniform mat4 MVP;

in vec3 vertexPos;
in vec2 texCoord;

out vec2 texCoord_v;

void main()
{
	texCoord_v = texCoord;
	gl_Position = MVP * vec4(vertexPos, 1);
}
