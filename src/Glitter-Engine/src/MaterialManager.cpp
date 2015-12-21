#include <GE/MaterialManager.hpp>

#include <GE/GraphicsEngine.hpp>

#include <iostream>
#include <fstream>

Material* MaterialManager::LoadStandardMaterialFromFile(GraphicsEngine* pGraphicsEngine, const std::string& filename, const std::string& materialName)
{
	const std::string materialKey = filename + "?" + materialName;

	auto elementIter = materials.find(materialKey);
	if (elementIter != materials.end()) {
		return (*elementIter).second;
	}

	std::cout << "Loading material..." << std::endl;

	StandardMaterial *pResult  = LoadFromMTLFile(pGraphicsEngine, filename, materialName);

	std::cout << "Material loaded!" << std::endl;

	materials[filename] = pResult;

	return pResult;
}

StandardMaterial* MaterialManager::LoadFromMTLFile(GraphicsEngine* pGraphicsEngine, const std::string& filename, const std::string& materialName)
{
	StandardMaterial *pResult = new StandardMaterial(pGraphicsEngine);

	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("Could not open material file: ") + filename);
	}

	std::string read;
	std::string currMaterial;
	std::string texFilename;

	//Skip to requested material
	bool found = false;
	while (file >> read && !found) {
		if (read == "newmtl") {
			file >> currMaterial;
			//std::getline(file, currMaterial);
			if (currMaterial == materialName) {
				found = true;
			}
		}
		else {
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	if (!found) {
		throw std::runtime_error(std::string("Could not find material \"") + materialName + "\" in file: " + filename);
	}

	sf::Image normalImg;
	sf::Image specularImg;
	sf::Image roughnessImg;

	//Initialize normal map
	{
		std::vector<sf::Uint8> pixelData = { 128, 128, 255, 255 };
		sf::Image img;
		img.create(1, 1, pixelData.data());

		normalImg = std::move(img);
	}

	bool readingTargetMaterial = true;
	while (file >> read && readingTargetMaterial) {
		if (read == "map_Kd") {
			file >> texFilename;

			sf::Image img;
			if (!img.loadFromFile(GraphicsEngine::GetImgFolderPath() + texFilename)) {
				throw std::runtime_error(filename);
			}
			img.flipVertically();

			const sf::Uint8 treshold = 250;
			bool &transparent = pResult->isTransparent;
			transparent = false;
			for (size_t x = 0; x < img.getSize().x && !transparent; x++) {
				for (size_t y = 0; y < img.getSize().y && !transparent; y++) {
					if (img.getPixel(x, y).a < treshold) {
						pResult->isTransparent = true;
					}
				}
			}

			LoadTexture(pResult->albedoTexture, img, TextureType::COLOR);
		}
		else if (read == "map_Ks") {
			file >> texFilename;

			specularImg = LoadImage(GraphicsEngine::GetImgFolderPath() + texFilename);
			//LoadTexture(pResult->specularTexture, GraphicsEngine::GetImgFolderPath() + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Ns") {
			file >> texFilename;

			//TODO map_Ns is the the inverse of roughness!!
			roughnessImg = LoadImage(GraphicsEngine::GetImgFolderPath() + texFilename);
			//LoadTexture(pResult->roughnessTexture, GraphicsEngine::GetImgFolderPath() + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Bump" || read == "map_bump" || read == "bump") {
			file >> texFilename;

			normalImg = LoadImage(GraphicsEngine::GetImgFolderPath() + texFilename);
			//LoadTexture(pResult->normalMap, GraphicsEngine::GetImgFolderPath() + texFilename, TextureType::DATA);
		}
		else if (read == "newmtl") {
			readingTargetMaterial = false;
		}
		else {
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	//merge the normal, specular, and roughness together!
	const int normalW = normalImg.getSize().x;
	const int normalH = normalImg.getSize().y;

	if (specularImg.getSize() == sf::Vector2u(0, 0)) {
		specularImg.create(normalW, normalH);
	}
	if (roughnessImg.getSize() == sf::Vector2u(0, 0)) {
		roughnessImg.create(normalW, normalH);
	}

	const bool sizes_match = (normalImg.getSize() == specularImg.getSize()) && (normalImg.getSize() == roughnessImg.getSize());
	if (!sizes_match) {
		throw std::runtime_error("Normal map, specular map, and roughness map must have the same size. ("+filename+")");
	}

	sf::Image normal_spec_rough_img;
	normal_spec_rough_img.create(normalW, normalH);

	for (size_t x = 0; x < normal_spec_rough_img.getSize().x; x++) {
		for (size_t y = 0; y < normal_spec_rough_img.getSize().y; y++) {
			sf::Color normal = normalImg.getPixel(x, y);
			sf::Uint8 spec = specularImg.getPixel(x, y).r;
			sf::Uint8 roughness = roughnessImg.getPixel(x, y).r;
			normal_spec_rough_img.setPixel(x, y, sf::Color(normal.r, normal.g, spec, roughness));
		}
	}

	LoadTexture(pResult->normal_spec_rough_Texture, normal_spec_rough_img, TextureType::DATA);

	return pResult;
}

void MaterialManager::LoadTexture(gl::Texture& target, const std::string& filename, TextureType type)
{
	sf::Image img;

	if (!img.loadFromFile(filename)){
		throw std::runtime_error(filename);
	}

	img.flipVertically();

	LoadTexture(target, img, type);
}

void MaterialManager::LoadTexture(gl::Texture& target, const sf::Image& img, TextureType type)
{
	target.Bind(gl::Texture::Target::_2D);
	gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::Linear);
	gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);
	gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);
	gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);

	gl::Texture::Image2D(gl::Texture::Target::_2D,
		0,
		type == TextureType::COLOR ? gl::enums::PixelDataInternalFormat::SRGB8Alpha8 : gl::enums::PixelDataInternalFormat::RGBA,
		img.getSize().x,
		img.getSize().y,
		0,
		gl::enums::PixelDataFormat::RGBA,
		gl::DataType::UnsignedByte,
		img.getPixelsPtr());
}

sf::Image MaterialManager::LoadImage(const std::string& filename)
{
	sf::Image img;

	if (!img.loadFromFile(filename)){
		throw std::runtime_error(filename);
	}

	img.flipVertically();

	return img;
}
