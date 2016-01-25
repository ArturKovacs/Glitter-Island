#version 330

uniform samplerCube cubeMap;

uniform float multiplyer;

in vec3 texCoordFromVert;
out vec4 fragColor;

void main(void)
{
	fragColor = vec4(texture(cubeMap, normalize(texCoordFromVert)).rgb*multiplyer, clamp(texCoordFromVert.y+1, 0, 1));
}
