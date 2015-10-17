#pragma once

#include "Material.hpp"
#include "all_gl_headers.hpp"
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

	StandardMaterial(StandardMaterial&&);
	StandardMaterial& operator=(StandardMaterial&&);

	void Prepare(Mesh::Submesh& submsh, const gl::Mat4f& modelTransform) override;
	void Prepare(Mesh::Submesh& submsh) override;
	void SetTransform(const gl::Mat4f& modelTransform) override;

private:
	GraphicsEngine* pGraphicsEngine;

	gl::Texture albedoTexture;
	gl::Texture normalMap;
	gl::Texture specularTexture;
	gl::Texture roughnessTexture;

	gl::Uniform<gl::Vec3f> sh_lightDir;
	gl::Uniform<gl::Mat4f> sh_MVP;
	gl::Uniform<gl::Mat4f> sh_MODELVIEW;
	gl::Uniform<gl::Mat4f> sh_modelTransposedInverse;

	gl::Program shaderProgram;
};

