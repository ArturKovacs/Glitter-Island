#version 330

uniform mat4 MVP;
uniform mat4 MODELVIEW;
uniform mat4 MODEL_tr_inv;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec3 vertexTangent;

out vec3 normal_v;
out vec2 texCoord_v;
out vec3 tangent_v;
out vec3 viewerDir_v;
out float viewZ_v;

void main(void)
{
	texCoord_v = vertexTexCoord;
	normal_v = (MODEL_tr_inv * vec4(vertexNormal, 0)).xyz;
	tangent_v = (MODEL_tr_inv * vec4(vertexTangent, 0)).xyz;
	gl_Position = MVP * vec4(vertexPos, 1.0);
	
	vec3 viewPos = (MODELVIEW * vec4(vertexPos, 1.0)).xyz;
	viewZ_v = viewPos.z;
	viewerDir_v = (-viewPos).xyz;
}
