#version 330

uniform mat4 MVP;

in vec3 pos;
in vec3 normal;
in vec2 texCoord;

out vec3 normal_v;
out vec2 texCoord_v;

void main()
{
	gl_Position = MVP * vec4(pos, 1);
	
	normal_v = normal;
	texCoord_v = texCoord;
}
