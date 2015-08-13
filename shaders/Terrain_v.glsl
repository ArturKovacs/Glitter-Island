#version 330

uniform mat4 MVP;
uniform mat4 model_transposed_inverse;
uniform mat4 modelTransform;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

out vec3 normal_v;
out vec2 texCoord_v;
out vec3 worldPos_v;

void main(void)
{
	texCoord_v = vertexTexCoord;
	normal_v = vec3(model_transposed_inverse * vec4(vertexNormal, 0));
	vec4 worldPosTmp = modelTransform * vec4(vertexPos, 1);
	worldPos_v = (worldPosTmp / worldPosTmp.w).xyz;
	gl_Position = MVP * vec4(vertexPos, 1.0);
}
