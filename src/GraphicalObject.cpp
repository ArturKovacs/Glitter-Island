#include "GraphicalObject.hpp"

#include "DemoCore.hpp"

GraphicalObject::GraphicalObject()
{
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

	//LoadTexture(albedoTexture, DemoCore::imgFolderPath + "Cerberus_A.tga", TextureType::COLOR);
	//LoadTexture(normalMap, DemoCore::imgFolderPath + "Cerberus_N.tga", TextureType::DATA);
	//LoadTexture(specularTexture, DemoCore::imgFolderPath + "Cerberus_M.tga", TextureType::DATA);
	//LoadTexture(roughnessTexture, DemoCore::imgFolderPath + "Cerberus_R.tga", TextureType::DATA);
}

GraphicalObject::GraphicalObject(GraphicalObject&& r):
mesh(std::move(r.mesh)),
albedoTexture(std::move(r.albedoTexture)),
normalMap(std::move(r.normalMap)),
specularTexture(std::move(r.specularTexture)),
roughnessTexture(std::move(r.roughnessTexture)),
sh_lightDir(std::move(r.sh_lightDir)),
sh_MVP(std::move(r.sh_MVP)),
sh_MODELVIEW(std::move(r.sh_MODELVIEW)),
sh_modelTransposedInverse(std::move(r.sh_modelTransposedInverse)),
shaderProgram(std::move(r.shaderProgram)),
modelTransform(std::move(r.modelTransform))
{}

GraphicalObject& GraphicalObject::operator=(GraphicalObject&& r)
{
	mesh = std::move(r.mesh);
	albedoTexture = std::move(r.albedoTexture);
	normalMap = std::move(r.normalMap);
	specularTexture = std::move(r.specularTexture);
	roughnessTexture = std::move(r.roughnessTexture);
	sh_lightDir = std::move(r.sh_lightDir);
	sh_MVP = std::move(r.sh_MVP);
	sh_MODELVIEW = std::move(r.sh_MODELVIEW);
	sh_modelTransposedInverse = std::move(r.sh_modelTransposedInverse);
	shaderProgram = std::move(r.shaderProgram);
	modelTransform = std::move(r.modelTransform);

	return *this;
}

void GraphicalObject::SetMesh(Mesh&& newMesh)
{
	mesh = std::move(newMesh);

	mesh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
	mesh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
	mesh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");
	mesh.AttachVertexAttribute(AttributeCategory::TANGENT, shaderProgram, "vertexTangent");
}

Mesh& GraphicalObject::GetMesh()
{
	return mesh;
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

void GraphicalObject::Draw(DemoCore& core)
{
	auto& glContext = core.GetGLContext();

	shaderProgram.Use();

	if (sh_MVP.IsActive()) {
		sh_MVP.Set(core.GetCamera().GetViewProjectionTransform() * modelTransform);
	}
	if (sh_modelTransposedInverse.IsActive()) {
		sh_modelTransposedInverse.Set(SajatTransposeMertNemMukodikAzOglPlusOsTODO(gl::Inverse(modelTransform)));
	}
	if (sh_lightDir.IsActive()) {
		sh_lightDir.Set(core.GetSun().GetDirectionTowardsSource());
	}

	gl::Texture::Active(0);
	albedoTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(1);
	normalMap.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(2);
	specularTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(3);
	roughnessTexture.Bind(gl::Texture::Target::_2D);

	mesh.BindVAO();
	glContext.DrawElements(mesh.GetPrimitiveType(), mesh.GetNumOfIndices(), mesh.indexTypeEnum);
}

void GraphicalObject::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f GraphicalObject::GetTransform() const
{
	return modelTransform;
}

void GraphicalObject::LoadMaterial(const std::string& filename)
{
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("Could not open material file: ") + filename);
	}

	std::string read;
	std::string texFilename;

	while (file >> read) {
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

			LoadTexture(normalMap, DemoCore::imgFolderPath + texFilename, TextureType::COLOR);
		}
		else {
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
}

void GraphicalObject::LoadTexture(gl::Texture& target, const std::string& filename, TextureType type)
{
	sf::Image img;

	if (!img.loadFromFile(filename)){
		throw std::runtime_error(filename);
	}

	img.flipVertically();

	target.Bind(gl::Texture::Target::_2D);
	gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::Linear);
	gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);
	gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::ClampToEdge);
	gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::ClampToEdge);

	gl::Texture::Image2D(gl::Texture::Target::_2D,
		0,
		type == TextureType::COLOR ? gl::enums::PixelDataInternalFormat::SRGB8 : gl::enums::PixelDataInternalFormat::RGB,
		img.getSize().x,
		img.getSize().y,
		0,
		gl::enums::PixelDataFormat::RGBA,
		gl::DataType::UnsignedByte,
		img.getPixelsPtr());
}
