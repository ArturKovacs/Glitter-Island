#pragma once

#include "Material.hpp"

class DepthOnlyMaterial : public Material
{
public:
	DepthOnlyMaterial(DemoCore* pCore);
	~DepthOnlyMaterial();

	const gl::Texture* GetTextureContainigAlpha() const override;
	void Prepare(Mesh::Submesh& submsh, gl::Mat4f& modelTransform) override;

	void SetTextureContainingAlpha(const gl::Texture* pTexture);
	void ResetTextureContainingAlpha();

private:
	DemoCore* pCore;

	gl::Texture defaultTexture;
	const gl::Texture* pTextureContainingAlpha;

	gl::Uniform<gl::Mat4f> sh_MVP;

	gl::Program shaderProgram;
};
