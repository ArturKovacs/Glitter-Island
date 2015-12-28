#pragma once

#include <GE/Material.hpp>
#include <GE/all_gl_headers.hpp>
#include <SFML/Graphics.hpp>

class GraphicsEngine;

class StandardMaterial : public Material
{
	friend class MaterialManager;

public:
	StandardMaterial(GraphicsEngine* pGraphicsEngine);
	~StandardMaterial();

	StandardMaterial(const StandardMaterial&) = delete;
	StandardMaterial& operator=(const StandardMaterial&) = delete;

	StandardMaterial(StandardMaterial&&) = default;
	StandardMaterial& operator=(StandardMaterial&&) = default;

	virtual void Prepare(Mesh::Submesh& submsh, const glm::mat4& modelTransform) override;
	virtual void Prepare(Mesh::Submesh& submsh) override;
	virtual void SetTransform(const glm::mat4& modelTransform) override;

protected:
	GraphicsEngine* pGraphicsEngine;

	bool isTransparent;

	gl::Texture albedoTexture;
	gl::Texture normal_spec_rough_Texture;
	//gl::Texture normalMap;
	//gl::Texture specularTexture;
	//gl::Texture roughnessTexture;

	gl::Uniform<glm::vec3> sh_lightDir;
	gl::Uniform<glm::mat4> sh_MVP;
	gl::Uniform<glm::mat4> sh_MODELVIEW;
	gl::Uniform<glm::mat4> sh_modelTrInv;

	gl::Program shaderProgram;
};

