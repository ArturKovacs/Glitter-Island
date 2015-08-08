#pragma once

#include "Material.hpp"
#include "StandardMaterial.hpp"
#include <map>

class DemoCore;

class MaterialManager
{
public:
	Material* LoadStandardMaterialFromFile(DemoCore* pCore, const std::string& filename, const std::string& materialName);
private:
	std::map<std::string, Material*> materials;

private:
	StandardMaterial* LoadFromMTLFile(DemoCore* pCore, const std::string& filename, const std::string& materialName);

private:
	enum class TextureType { COLOR, DATA };
	static void LoadTexture(gl::Texture& target, const std::string& filename, TextureType type);
	static void LoadTexture(gl::Texture& target, const sf::Image& img, TextureType type);
};
