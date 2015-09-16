#include <GE/MaterialManager.hpp>

#include <GE/GraphicsEngine.hpp>

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

	//Initialize normal map
	{
		std::vector<sf::Uint8> pixelData = { 128, 128, 255, 255 };
		sf::Image img;
		img.create(1, 1, pixelData.data());

		LoadTexture(pResult->normalMap, img, TextureType::DATA);
	}

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

	bool readingTargetMaterial = true;
	while (file >> read && readingTargetMaterial) {
		if (read == "map_Kd") {
			file >> texFilename;

			LoadTexture(pResult->albedoTexture, GraphicsEngine::GetImgFolderPath() + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Ks") {
			file >> texFilename;

			LoadTexture(pResult->specularTexture, GraphicsEngine::GetImgFolderPath() + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Ns") {
			file >> texFilename;

			//TODO map_Ns is the the inverse of roughness!!
			LoadTexture(pResult->roughnessTexture, GraphicsEngine::GetImgFolderPath() + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Bump" || read == "map_bump" || read == "bump") {
			file >> texFilename;

			LoadTexture(pResult->normalMap, GraphicsEngine::GetImgFolderPath() + texFilename, TextureType::DATA);
		}
		else if (read == "newmtl") {
			readingTargetMaterial = false;
		}
		else {
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

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
