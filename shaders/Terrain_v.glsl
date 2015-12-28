#version 330

uniform mat4 MVP;
uniform mat4 MODEL_tr_inv;
uniform mat4 MODEL;
uniform mat4 MODELVIEW;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

out vec3 normal_v;
out vec2 texCoord_v;
out vec3 worldPos_v;
out float viewZ_v;

void main(void)
{
	texCoord_v = vertexTexCoord;
	normal_v = vec3(MODEL_tr_inv * vec4(vertexNormal, 0));
	worldPos_v = (MODEL * vec4(vertexPos, 1)).xyz;
	viewZ_v = (MODELVIEW * vec4(vertexPos, 1)).z;
	gl_Position = MVP * vec4(vertexPos, 1.0);
}
