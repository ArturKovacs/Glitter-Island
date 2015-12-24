#pragma once

#include <map>
#include <SFML/Graphics.hpp>
#include <GE/Material.hpp>
#include <GE/Utility.hpp>
//#include "StandardMaterial.hpp"

class GraphicsEngine;

class MaterialManager
{
public:
	util::managed_ptr<Material> LoadFromFile(GraphicsEngine* pGraphicsEngine, const std::string& filename, const std::string& materialName);

private:
	enum class TextureType { COLOR, DATA };
	static void LoadTexture(gl::Texture& target, const std::string& filename, TextureType type);
	static void LoadTexture(gl::Texture& target, const sf::Image& img, TextureType type);
	static sf::Image LoadImage(const std::string& filename);

private:
	using materialIdentifier = std::pair<std::string, std::string>;
	std::map<materialIdentifier, Material*> materials;

private:
	Material* LoadFromMTLFile(GraphicsEngine* pGraphicsEngine, const std::string& filename, const std::string& materialName);
};
