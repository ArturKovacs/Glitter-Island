#version 330

uniform mat4 MVP;

in vec3 vertexPos;
out vec3 posFromVert;

void main(void)
{
	//vec3 modifiedVertexPos = vec3(vertexPos.x, vertexPos.y+(cos(vertexPos.x)+cos(vertexPos.z))*1, vertexPos.z);
	
	//invViewProj = inverse(viewProj);
	
	gl_Position = MVP * vec4(vertexPos, 1.0);
	posFromVert = vertexPos;
}
