#pragma once

#include "Material.hpp"

class StandardMaterial;

#include "all_gl_headers.hpp"
#include "DemoCore.hpp"

class StandardMaterial : public Material
{
	friend Material* DemoCore::LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName);

public:
	StandardMaterial(DemoCore* pDemoCore);
	~StandardMaterial();

	StandardMaterial(const GraphicalObject&) = delete;
	StandardMaterial& operator=(const StandardMaterial&) = delete;

	StandardMaterial(StandardMaterial&&);
	StandardMaterial& operator=(StandardMaterial&&);

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

private:
	enum class TextureType { COLOR, DATA };
	static void LoadTexture(gl::Texture& target, const std::string& filename, TextureType type);
	static void LoadTexture(gl::Texture& target, const sf::Image& img, TextureType type);

private:
	void LoadFromMTLFile(const std::string& filename, const std::string& materialName);
};

