#version 330

uniform mat4 MVP;
//uniform mat4 invMVP;

in vec3 vertexPos;
out vec3 posFromVert;

flat out mat4 invMVP;

void main(void)
{
	//vec3 modifiedVertexPos = vec3(vertexPos.x, vertexPos.y+(cos(vertexPos.x)+cos(vertexPos.z))*1, vertexPos.z);
	
	invMVP = inverse(MVP);
	
	gl_Position = MVP * vec4(vertexPos, 1.0);
	posFromVert = vertexPos;
}
