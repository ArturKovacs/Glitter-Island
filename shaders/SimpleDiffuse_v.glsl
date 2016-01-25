#version 330

uniform mat4 MVP;
uniform mat4 MODEL_tr_inv;

in vec3 vertexPos;
in vec3 vertexNormal;

out vec3 normal_v;

void main(void)
{
	normal_v = (MODEL_tr_inv * vec4(vertexNormal, 0)).xyz;
	gl_Position = MVP * vec4(vertexPos, 1.0);
}
