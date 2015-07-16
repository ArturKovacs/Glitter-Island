#version 330

uniform mat4 transposed_inverse_model;

uniform sampler2D diffTexture;
uniform sampler2D smoothnessTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

in vec3 normal_v;
in vec2 texCoord_v;

//TODO actualy all buffers are 32bit total. (might not need extra care)
out vec4 abledo_smooth;
out vec4 spec_emiss;
out vec2 normalXY;

void main()
{
	//TODO sample texture and write values to outputs.
	abledo_smooth.rgb = texture(diffTexture, texCoord_v).rgb;
	albedo_smooth.a = texture(smoothnessTexture, texCoord_v).r;
	
	spec_emiss.rbg = texture(specularTexture, texCoord_v).rgb;
	spec_emiss.a = 0; //TODO emission
	
	normalXY = texture(normalTexture, texCoord_v).xy;
}
