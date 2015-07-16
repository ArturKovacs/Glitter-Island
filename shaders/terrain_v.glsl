#version 330

uniform mat4 MVP;
uniform mat4 model_transposed_inverse;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

out vec3 normalFromVert;
out vec2 texCoordFromVert;

void main(void)
{
	texCoordFromVert = vertexTexCoord;
	normalFromVert = vec3(model_transposed_inverse * vec4(vertexNormal, 0));
	//normalFromVert = vertexNormal;
	gl_Position = MVP * vec4(vertexPos, 1.0);
}
