#include <GE/StandardMaterial.hpp>

#include <GE/GraphicsEngine.hpp>
#include <iostream>

StandardMaterial::StandardMaterial(GraphicsEngine* pGraphicsEngine) : pGraphicsEngine(pGraphicsEngine)
{}

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
	sh_lightColor.Set(pGraphicsEngine->GetSun().GetColor());

	gl::Texture::Active(0);
	albedoTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(1);
	normal_spec_rough_Texture.Bind(gl::Texture::Target::_2D);

	if (isTransparent) {
		pGraphicsEngine->GetGLContext().Disable(gl::enums::Capability::CullFace);
	}
	else {
		pGraphicsEngine->GetGLContext().Enable(gl::enums::Capability::CullFace);
	}
}

void StandardMaterial::SetTransform(const glm::mat4& modelTransform)
{
	shaderProgram.Use();

	sh_MVP.Set(pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
	sh_MODELVIEW.Set(pGraphicsEngine->GetActiveCamera()->GetViewTransform() * modelTransform);
	sh_modelTrInv.Set(glm::transpose(glm::inverse(modelTransform)));
}
