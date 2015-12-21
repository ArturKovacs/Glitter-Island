#include <GE/StandardMaterial.hpp>

#include <GE/GraphicsEngine.hpp>
#include <iostream>

StandardMaterial::StandardMaterial(GraphicsEngine* pGraphicsEngine) : pGraphicsEngine(pGraphicsEngine)
{
	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("StandardMaterial_v.glsl", "StandardMaterial_f.glsl");
	shaderProgram.Use();

	isTransparent = false;

	try {
		sh_MVP = gl::Uniform<glm::mat4>(shaderProgram, "MVP");
		sh_MODELVIEW = gl::Uniform<glm::mat4>(shaderProgram, "MODELVIEW");
		sh_modelTransposedInverse = gl::Uniform<glm::mat4>(shaderProgram, "model_transposed_inverse");
		sh_lightDir = gl::Uniform<glm::vec3>(shaderProgram, "lightDir");
		gl::UniformSampler(shaderProgram, "albedoTexture").Set(0);
		gl::UniformSampler(shaderProgram, "normal2_spec1_rough1_Tex").Set(1);
		//gl::UniformSampler(shaderProgram, "normalMap").Set(1);
		//gl::UniformSampler(shaderProgram, "specularTexture").Set(2);
		//gl::UniformSampler(shaderProgram, "roughnessTexture").Set(3);
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}
}

StandardMaterial::~StandardMaterial()
{}

void StandardMaterial::Prepare(Mesh::Submesh& submsh, const glm::mat4& modelTransform)
{
	Prepare(submsh);
	SetTransform(modelTransform);
}

void StandardMaterial::Prepare(Mesh::Submesh& submsh)
{
	//TODO might be too slow to set vertex attributes every time,
	//but if multiple meshes are drawn with a single  material it is necessary
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
	submsh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
	submsh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");
	submsh.AttachVertexAttribute(AttributeCategory::TANGENT, shaderProgram, "vertexTangent");

	shaderProgram.Use();

	sh_lightDir.Set(pGraphicsEngine->GetSun().GetDirectionTowardsSource());

	gl::Texture::Active(0);
	albedoTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(1);
	normal_spec_rough_Texture.Bind(gl::Texture::Target::_2D);

	if (isTransparent) {
		pGraphicsEngine->GetGLContext().Disable(gl::Capability::CullFace);
	}
	else {
		pGraphicsEngine->GetGLContext().Enable(gl::Capability::CullFace);
	}
}

void StandardMaterial::SetTransform(const glm::mat4& modelTransform)
{
	shaderProgram.Use();

	sh_MVP.Set(pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
	sh_modelTransposedInverse.Set(glm::transpose(glm::inverse(modelTransform)));
}
