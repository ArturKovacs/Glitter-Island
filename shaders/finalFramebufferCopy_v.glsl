#version 330

uniform int screenWidth;
uniform int screenHeight;

in vec2 vertexPos;

out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(vertexPos, 0, 1.0);
	textureCoords = vec2(
						(vertexPos.x*0.5 + 0.5)*screenWidth,
						(vertexPos.y*0.5 + 0.5)*screenHeight);
}
