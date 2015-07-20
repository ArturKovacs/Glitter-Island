#version 330

uniform mat4 MVP;

in vec2 vertexPos;

void main()
{
	gl_Position = MVP * vec4(vertexPos, 0, 1);
}
