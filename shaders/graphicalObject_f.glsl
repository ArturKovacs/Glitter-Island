#version 330

uniform vec3 lightDir;
uniform sampler2D normalMap;
uniform sampler2D albedoTexture;

in vec2 texCoord_v;
in vec3 normal_v;
in vec3 tangent_v;

out vec4 fragColor;

//vec3 lightDir = normalize(vec3(-3, 2, 2));

void main(void) 
{ 
	vec3 normal = normalize(normal_v);
	vec3 tangent  = normalize(tangent_v);
	tangent = normalize(tangent - normal * (dot(normal, tangent)));
	//vec3 bitangent = cross(normal, tangent);
	vec3 bitangent = cross(tangent, normal);
	
	mat3 textureToWorld = mat3(tangent, bitangent, normal);
	
	normal = normalize(textureToWorld * (texture(normalMap, texCoord_v).xyz*2.0-1.0));
	
	//normal = (texture(normalMap, texCoord_v).xyz*2.0-1.0);
	
	fragColor = vec4(texture(albedoTexture, texCoord_v).xyz * max(dot(lightDir, normal), 0), 1.0); 
	//fragColor = vec4(normal, 1.0);
} 
