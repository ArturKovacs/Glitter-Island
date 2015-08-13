#pragma once

#include "Material.hpp"
#include "all_gl_headers.hpp"
#include <SFML/Graphics.hpp>

class StandardMaterial : public Material
{
	friend class MaterialManager;

public:
	StandardMaterial(DemoCore* pCore);
	~StandardMaterial();

	StandardMaterial(const StandardMaterial&) = delete;
	StandardMaterial& operator=(const StandardMaterial&) = delete;

	StandardMaterial(StandardMaterial&&);
	StandardMaterial& operator=(StandardMaterial&&);

	const gl::Texture* GetTextureContainigAlpha() const override;
	void Prepare(Mesh::Submesh& submsh, gl::Mat4f& modelTransform) override;

private:
	DemoCore* pCore;

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

