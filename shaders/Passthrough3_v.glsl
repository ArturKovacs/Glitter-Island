#version 330

in vec3 vertexPos;

void main()
{
	gl_Position = vec4(vertexPos, 1);
}
