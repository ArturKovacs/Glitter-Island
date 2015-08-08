#version 330

uniform mat4 MVP;
uniform mat4 MODELVIEW;
uniform mat4 model_transposed_inverse;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec3 vertexTangent;

smooth out vec3 normal_v;
out vec2 texCoord_v;
out vec3 tangent_v;

out vec3 viewerDir_v;

void main(void)
{
	texCoord_v = vertexTexCoord;
	normal_v = (model_transposed_inverse * vec4(vertexNormal, 0)).xyz;
	tangent_v = (model_transposed_inverse * vec4(vertexTangent, 0)).xyz;
	gl_Position = MVP * vec4(vertexPos, 1.0);
	
	viewerDir_v = ((MODELVIEW * vec4(vertexPos, 1.0))*-1).xyz;
}
