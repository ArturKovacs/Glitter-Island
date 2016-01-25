#version 330

uniform vec3 sunDir;
uniform vec3 sunColor;

uniform vec4 color;

in vec3 normal_v;

out vec4 fragColor;

void main(void) 
{ 
	fragColor = vec4(color.rgb * sunColor * max(dot(normalize(normal_v), sunDir), 0), color.a);
}
