#include "StandardMaterial.hpp"

#include "DemoCore.hpp"

StandardMaterial::StandardMaterial(DemoCore* pDemoCore) : pCore(pDemoCore)
{
	//TODO shader loading might be better to be done by DemoCore too.
	shaderProgram = DemoCore::LoadShaderProgramFromFiles("graphicalObject_v.glsl", "graphicalObject_f.glsl");
	shaderProgram.Use();

	try {
		sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
		sh_MODELVIEW = gl::Uniform<gl::Mat4f>(shaderProgram, "MODELVIEW");
		sh_modelTransposedInverse = gl::Uniform<gl::Mat4f>(shaderProgram, "model_transposed_inverse");
		sh_lightDir = gl::Uniform<gl::Vec3f>(shaderProgram, "lightDir");
		gl::UniformSampler(shaderProgram, "albedoTexture").Set(0);
		gl::UniformSampler(shaderProgram, "normalMap").Set(1);
		gl::UniformSampler(shaderProgram, "specularTexture").Set(2);
		gl::UniformSampler(shaderProgram, "roughnessTexture").Set(3);
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	std::vector<sf::Uint8> pixelData = { 128, 128, 255, 255 };
	sf::Image img;
	img.create(1, 1, pixelData.data());

	LoadTexture(normalMap, img, TextureType::DATA);
}

StandardMaterial::~StandardMaterial()
{}

StandardMaterial::StandardMaterial(StandardMaterial&& r) :
pCore(r.pCore),
albedoTexture(std::move(r.albedoTexture)),
normalMap(std::move(r.normalMap)),
specularTexture(std::move(r.specularTexture)),
roughnessTexture(std::move(r.roughnessTexture)),
sh_lightDir(std::move(r.sh_lightDir)),
sh_MVP(std::move(r.sh_MVP)),
sh_MODELVIEW(std::move(r.sh_MODELVIEW)),
sh_modelTransposedInverse(std::move(r.sh_modelTransposedInverse)),
shaderProgram(std::move(r.shaderProgram))
{}

StandardMaterial& StandardMaterial::operator=(StandardMaterial&& r)
{
	pCore = r.pCore;
	albedoTexture = std::move(r.albedoTexture);
	normalMap = std::move(r.normalMap);
	specularTexture = std::move(r.specularTexture);
	roughnessTexture = std::move(r.roughnessTexture);
	sh_lightDir = std::move(r.sh_lightDir);
	sh_MVP = std::move(r.sh_MVP);
	sh_MODELVIEW = std::move(r.sh_MODELVIEW);
	sh_modelTransposedInverse = std::move(r.sh_modelTransposedInverse);
	shaderProgram = std::move(r.shaderProgram);

	return *this;
}

static gl::Mat4f SajatTransposeMertNemMukodikAzOglPlusOsTODO(const gl::Mat4f& input)
{
	gl::Mat4f result;

	for (std::size_t i = 0; i != 4; ++i) {
		for (std::size_t j = 0; j != 4; ++j) {
			result.Set(i, j, input.At(j, i));
		}
	}

	return result;
}

void StandardMaterial::Prepare(Mesh::Submesh& submsh, gl::Mat4f& modelTransform)
{
	//TODO might be too slow to set vertex attributes every time, 
	//but if multiple meshes are drawn with a single  material it is necessary
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
	submsh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
	submsh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");
	submsh.AttachVertexAttribute(AttributeCategory::TANGENT, shaderProgram, "vertexTangent");

	shaderProgram.Use();

	if (sh_MVP.IsActive()) {
		sh_MVP.Set(pCore->GetCamera().GetViewProjectionTransform() * modelTransform);
	}
	if (sh_modelTransposedInverse.IsActive()) {
		sh_modelTransposedInverse.Set(SajatTransposeMertNemMukodikAzOglPlusOsTODO(gl::Inverse(modelTransform)));
	}
	if (sh_lightDir.IsActive()) {
		sh_lightDir.Set(pCore->GetSun().GetDirectionTowardsSource());
	}

	gl::Texture::Active(0);
	albedoTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(1);
	normalMap.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(2);
	specularTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(3);
	roughnessTexture.Bind(gl::Texture::Target::_2D);
}

void StandardMaterial::LoadTexture(gl::Texture& target, const std::string& filename, TextureType type)
{
	sf::Image img;

	if (!img.loadFromFile(filename)){
		throw std::runtime_error(filename);
	}

	img.flipVertically();

	LoadTexture(target, img, type);
}

void StandardMaterial::LoadTexture(gl::Texture& target, const sf::Image& img, TextureType type)
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

void StandardMaterial::LoadFromMTLFile(const std::string& filename, const std::string& materialName)
{
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

			LoadTexture(albedoTexture, DemoCore::imgFolderPath + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Ks") {
			file >> texFilename;

			LoadTexture(specularTexture, DemoCore::imgFolderPath + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Ns") {
			file >> texFilename;

			//TODO map_Ns is the the inverse of roughness!!
			LoadTexture(roughnessTexture, DemoCore::imgFolderPath + texFilename, TextureType::COLOR);
		}
		else if (read == "map_Bump" || read == "map_bump" || read == "bump") {
			file >> texFilename;

			LoadTexture(normalMap, DemoCore::imgFolderPath + texFilename, TextureType::DATA);
		}
		else if (read == "newmtl") {
			readingTargetMaterial = false;
		}
		else {
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
}
