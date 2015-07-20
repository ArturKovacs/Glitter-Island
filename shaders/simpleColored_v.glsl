#version 330

uniform mat4 MVP;

in vec3 vertexPos;

void main()
{
	gl_Position = MVP * vec4(vertexPos, 1);
}
