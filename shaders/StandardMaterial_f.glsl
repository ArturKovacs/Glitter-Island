#version 330

uniform vec3 lightDir;
uniform sampler2D albedoTexture;
uniform sampler2D normal2_spec1_rough1_Tex;
//uniform sampler2D specularTexture;
//uniform sampler2D roughnessTexture;

in vec2 texCoord_v;
in vec3 normal_v;
in vec3 tangent_v;

in vec3 viewerDir_v;

float PhongBlinn(const in vec3 lightSourceDir, const in vec3 viewerDir, const in vec3 normal, const in float shininess)
{
	vec3 halfway = normalize(lightSourceDir+viewerDir);
	return pow(max(dot(normal, halfway), 0), shininess);
}

//assuming that z is always positive
vec3 UnpackNormal(in vec2 packed)
{
	packed = packed*2-1;
	return normalize(vec3(packed, sqrt(-(packed.x*packed.x+packed.y*packed.y-1))));
}

void main(void) 
{
	vec4 kd = texture(albedoTexture, texCoord_v);
	if(kd.a < 0.9) { discard; }

	vec4 normal2_spec1_rough1 = texture(normal2_spec1_rough1_Tex, texCoord_v);
	
	float ks = normal2_spec1_rough1.z;
	//vec4 kd = texture(albedoTexture, texCoord_v) * (vec4(1) - vec4(ks, 0));
	vec3 ambient = kd.rgb * 0.02;

	vec3 normal = normalize(normal_v);
	vec3 tangent  = normalize(tangent_v);
	tangent = normalize(tangent - normal * dot(normal, tangent));
	//vec3 bitangent = cross(normal, tangent);
	vec3 bitangent = cross(tangent, normal);
	
	mat3 textureToWorld = mat3(tangent, bitangent, normal);
	
	normal = normalize(textureToWorld * UnpackNormal(normal2_spec1_rough1.xy));
	
	const float maxShininess = 256;
	float shininess = (1 - normal2_spec1_rough1.w) * maxShininess;
	
	gl_FragData[0] = vec4(kd.xyz*max(dot(lightDir, normal), 0) + ks*PhongBlinn(lightDir, normalize(viewerDir_v), normal, shininess) + ambient, 1);
	gl_FragData[1] = vec4(normal, 1);
	//fragColor = vec4((kd.xyz + ks*PhongBlinn(lightDir, normalize(viewerDir_v), normal, shininess)) * max(dot(lightDir, normal), 0) + ambient, kd.a);
} 
