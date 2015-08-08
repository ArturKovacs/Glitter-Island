#version 330

uniform vec3 lightDir;
uniform sampler2D albedoTexture;
uniform sampler2D normalMap;
uniform sampler2D specularTexture;
uniform sampler2D roughnessTexture;

in vec2 texCoord_v;
smooth in vec3 normal_v;
in vec3 tangent_v;

in vec3 viewerDir_v;

out vec4 fragColor;

//vec3 lightDir = normalize(vec3(-3, 2, 2));

float PhongBlinn(const in vec3 lightSourceDir, const in vec3 viewerDir, const in vec3 normal, const in float shininess)
{
	vec3 halfway = normalize(lightSourceDir+viewerDir);
	return pow(max(dot(normal, halfway), 0), shininess);
}

void main(void) 
{
	vec3 normal = normalize(normal_v);
	vec3 tangent  = normalize(tangent_v);
	tangent = normalize(tangent - normal * dot(normal, tangent));
	//vec3 bitangent = cross(normal, tangent);
	vec3 bitangent = cross(tangent, normal);
	
	mat3 textureToWorld = mat3(tangent, bitangent, normal);
	
	normal = normalize(textureToWorld * (texture(normalMap, texCoord_v).xyz*2.0-1.0));
	
	//normal = (texture(normalMap, texCoord_v).xyz*2.0-1.0);
	
	vec3 ks = texture(specularTexture, texCoord_v).xyz;
	vec4 kd = texture(albedoTexture, texCoord_v) * (vec4(1) - vec4(ks, 0));
	
	gl_FragDepth = mix(1.0, gl_FragCoord.z, kd.a);
	
	const float maxShininess = 256;
	float shininess = (1 - texture(roughnessTexture, texCoord_v).x) * maxShininess;
	
	fragColor = vec4((kd.xyz + ks*PhongBlinn(lightDir, normalize(viewerDir_v), normal, shininess)) * max(dot(lightDir, normal), 0), kd.a); 
	//fragColor = vec4(normal, 1.0);
} 
