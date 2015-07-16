#version 330

uniform mat4 viewProjectionMatrix;

in vec3 vertexPos;
out vec3 texCoordFromVert;

void main(void)
{
	gl_Position = viewProjectionMatrix * vec4(vertexPos * 10.0, 1.0);
	texCoordFromVert = vertexPos;
}
