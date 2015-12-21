#pragma once

#include "Material.hpp"
#include "StandardMaterial.hpp"
#include <map>

class GraphicsEngine;

class MaterialManager
{
public:
	Material* LoadStandardMaterialFromFile(GraphicsEngine* pGraphicsEngine, const std::string& filename, const std::string& materialName);
private:
	std::map<std::string, Material*> materials;

private:
	StandardMaterial* LoadFromMTLFile(GraphicsEngine* pGraphicsEngine, const std::string& filename, const std::string& materialName);

private:
	enum class TextureType { COLOR, DATA };
	static void LoadTexture(gl::Texture& target, const std::string& filename, TextureType type);
	static void LoadTexture(gl::Texture& target, const sf::Image& img, TextureType type);
	static sf::Image LoadImage(const std::string& filename);
};
