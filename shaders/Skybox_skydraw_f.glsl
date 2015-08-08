#version 330

uniform samplerCube cubeMap;

in vec3 texCoordFromVert;
out vec4 fragColor;

void main(void)
{
	fragColor = vec4(texture(cubeMap, normalize(texCoordFromVert)).rgb, clamp(texCoordFromVert.y+1, 0, 1));
}
