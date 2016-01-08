#include <GE/Terrain.hpp>

#include <GE/GraphicsEngine.hpp>

#include <SFML/Graphics.hpp>
#include <exception>
#include <cmath>
#include <string>
#include <iostream>

#define IGNORE_TRY(x) try{x;}catch(std::exception&ex){std::cout<<ex.what()<<std::endl;}

Terrain::Terrain(GraphicsEngine* pGraphicsEngine) : pGraphicsEngine(pGraphicsEngine)
{
	terrainScale = 0;

	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("Terrain_v.glsl", "Terrain_f.glsl");
	seabottomProgram = GraphicsEngine::LoadShaderProgramFromFiles("SimpleColored_v.glsl", "SimpleColored_f.glsl");

	try {
		shaderProgram.Use();
		sh_sunDir = gl::Uniform<glm::vec3>(shaderProgram, "sunDir");
		sh_sunColor = gl::Uniform<glm::vec3>(shaderProgram, "sunColor");
		sh_modelTransform = gl::Uniform<glm::mat4>(shaderProgram, "MODEL");
		sh_MVP = gl::Uniform<glm::mat4>(shaderProgram, "MVP");
		sh_modelTrInv = gl::Uniform<glm::mat4>(shaderProgram, "MODEL_tr_inv");
		sh_modelViewTransform = gl::Uniform<glm::mat4>(shaderProgram, "MODELVIEW");

		gl::UniformSampler(shaderProgram, "sandTexture").Set(0);
		gl::UniformSampler(shaderProgram, "sandNormalMap").Set(1);
		gl::UniformSampler(shaderProgram, "grassTexture").Set(2);
		gl::UniformSampler(shaderProgram, "grassNormalMap").Set(3);
		gl::UniformSampler(shaderProgram, "materialTexture").Set(4);

		sh_worldToShadowMap.resize(pGraphicsEngine->GetLightCascadeCount());
		sh_viewSubfrustumFarPlanesTexDepth.resize(pGraphicsEngine->GetLightCascadeCount());

		int currTextureID = 5;
		for (int i = 0; i < pGraphicsEngine->GetLightCascadeCount(); i++) {
			IGNORE_TRY(gl::UniformSampler(shaderProgram, "cascadeShadowMaps[" + std::to_string(i) + "]").Set(currTextureID++));
			IGNORE_TRY(sh_worldToShadowMap.at(i) = gl::Uniform<glm::mat4>(shaderProgram, "worldToShadowMap[" + std::to_string(i) + "]"));
			IGNORE_TRY(sh_viewSubfrustumFarPlanesTexDepth.at(i) = gl::Uniform<float>(shaderProgram, "viewSubfrustumFarPlanesTexDepth[" + std::to_string(i) + "]"));
		}

		seabottomProgram.Use();
		seabottom_MVP = gl::Uniform<glm::mat4>(seabottomProgram, "MVP");
		seabottom_color = gl::Uniform<glm::vec4>(seabottomProgram, "color");
		seabottom_color.Set(glm::vec4(0.78f, 0.78f, 0.66f, 1.0f));
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	materialMapFilename = GraphicsEngine::GetImgFolderPath() + "materialMap.png";

	LoadTexture(sandTexture, GraphicsEngine::GetImgFolderPath() + "sand_seamless.png", false, 4);
	LoadTexture(sandNormalMap, GraphicsEngine::GetImgFolderPath() + "sand_seamless_normal.png", true);
	LoadTexture(grassTexture, GraphicsEngine::GetImgFolderPath() + "grass_seamless.png", false, 4);
	LoadTexture(grassNormalMap, GraphicsEngine::GetImgFolderPath() + "grass_seamless_normal.png", true);
	materialMap = LoadTexture(materialTexture, materialMapFilename, true);
}

void Terrain::LoadFromHeightMap(const std::string& fileName, float scale, float heightMultiplyer, bool invertNormals)
{
	static const int maxHeight = 255;
	static const bool fromSRGB = false;

	terrainScale = scale;

	sf::Image image;

	if (!image.loadFromFile(fileName)) {
		throw std::runtime_error(std::string("Can not load file: ") + fileName);
	}

	image.flipVertically();

	const size_t imgWidth = image.getSize().x;
	const size_t imgHeight = image.getSize().y;

	if (imgWidth < 2 || imgHeight < 2) {
		throw std::runtime_error("Terrain heightmap must be at least 2 pixel big in each direction!");
	}

	const size_t pixelCount = imgWidth * imgHeight;
	const size_t totalVertexCount = pixelCount;

	std::vector<glm::vec3> positions = CalculatePositions(image, scale, heightMultiplyer);
	std::vector<glm::vec2> texCoords = CalculateTexCoords(image);
	std::vector<glm::vec3> normals = CalculateNormals(image, positions);
	std::vector<glm::vec3> tangent = CalculateTangents(image, positions, texCoords);

	const int pixelCountWithAssociatedQuad = (pixelCount-imgWidth-imgHeight)+1;
	const int trianglesInAQuad = 2;
	const int triangleVertexCount = 3;

	std::vector<Mesh::Submesh::IndexType> indices(pixelCountWithAssociatedQuad * trianglesInAQuad * triangleVertexCount);
	SetUpIndices(&indices, imgWidth, imgHeight);

	shaderProgram.Use();

	Mesh::Submesh submsh;

	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, positions);
	submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, normals);
	submsh.SetVertexAttributeBuffer(AttributeCategory::TANGENT, tangent);
	submsh.SetVertexAttributeBuffer(AttributeCategory::TEX_COORD, texCoords);

	submsh.SetIndices(indices);

	try {
		submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
		submsh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
		submsh.AttachVertexAttribute(AttributeCategory::TANGENT, shaderProgram, "vertexTangent");
		submsh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	submsh.SetPrimitiveType(gl::enums::PrimitiveType::Triangles);

	terrainModel.GetSubmeshes().push_back(std::move(submsh));

	const float seaBottomScale = scale*10;
	std::vector<GLfloat> seaBottomVertPos = {
		-seaBottomScale, -seaBottomScale, -0.1f,
		+seaBottomScale, -seaBottomScale, -0.1f,
		+seaBottomScale, +seaBottomScale, -0.1f,
		-seaBottomScale, +seaBottomScale, -0.1f
	};
	std::vector<GLfloat> seaBottomVertNormal = {
		0, 0, 1.f,
		0, 0, 1.f,
		0, 0, 1.f,
		0, 0, 1.f
	};
	std::vector<Mesh::Submesh::IndexType> seaBottomIndices = {
		0, 1, 2, 3
	};

	seabottomProgram.Use();
	Mesh::Submesh seabottom_submsh;

	seabottom_submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, seaBottomVertPos);
	seabottom_submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, seaBottomVertNormal);

	seabottom_submsh.SetIndices(seaBottomIndices);

	try {
		seabottom_submsh.AttachVertexAttribute(AttributeCategory::POSITION, seabottomProgram, "vertexPos");
		//seabottom_submsh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	seabottom_submsh.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);

	seabottom.GetSubmeshes().push_back(std::move(seabottom_submsh));
}

void Terrain::Draw()
{
	shaderProgram.Use();
	gl::Texture::Active(0); sandTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(1); sandNormalMap.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(2); grassTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(3); grassNormalMap.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(4); materialTexture.Bind(gl::Texture::Target::_2D);
	int currTextureID = 5;

	for (int i = 0; i < pGraphicsEngine->GetLightCascadeCount(); i++) {
		gl::Texture::Active(currTextureID++);
		pGraphicsEngine->GetCascadeShadowMap(i).Bind(gl::Texture::Target::_2D);

		if (sh_worldToShadowMap[i].IsActive()){
			sh_worldToShadowMap[i].Set(pGraphicsEngine->GetCascadeViewProjectionTransform(i));
		}
		if (sh_viewSubfrustumFarPlanesTexDepth[i].IsActive()) {
			sh_viewSubfrustumFarPlanesTexDepth[i].Set(pGraphicsEngine->GetViewSubfrustumFarPlaneInTexCoordZ(i));
		}
	}

	sh_sunDir.Set(pGraphicsEngine->GetSun().GetDirectionTowardsSource());
	sh_sunColor.Set(pGraphicsEngine->GetSun().GetColor());
	sh_modelTransform.Set(modelTransform);
	glm::mat4 MVP = pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform;
	sh_MVP.Set(MVP);
	glm::mat4 modelView = pGraphicsEngine->GetActiveCamera()->GetViewTransform() * modelTransform;
	sh_modelViewTransform.Set(modelView);
	sh_modelTrInv.Set(glm::transpose(glm::inverse(modelTransform)));

	pGraphicsEngine->GetGLContext().Enable(gl::Capability::CullFace);
	pGraphicsEngine->GetGLContext().Enable(gl::Capability::DepthTest);
	Mesh::Submesh& terrain_submsh = terrainModel.GetSubmeshes().at(0);
	terrain_submsh.BindVAO();
	pGraphicsEngine->GetGLContext().DrawElements(terrain_submsh.GetPrimitiveType(), terrain_submsh.GetNumOfIndices(), terrain_submsh.indexTypeEnum);

	seabottomProgram.Use();
	seabottom_MVP.Set(MVP);
	Mesh::Submesh& seabottom_submsh = seabottom.GetSubmeshes().at(0);
	seabottom_submsh.BindVAO();
	pGraphicsEngine->GetGLContext().DrawElements(seabottom_submsh.GetPrimitiveType(), seabottom_submsh.GetNumOfIndices(), seabottom_submsh.indexTypeEnum);
}

void Terrain::SetTransform(const glm::mat4& transform)
{
	modelTransform = transform;
}

glm::mat4 Terrain::GetTransform() const
{
	return modelTransform;
}

sf::Image& Terrain::GetMaterialMap()
{
	return materialMap;
}

glm::ivec2 Terrain::GetMaterialMapPos(const glm::vec4 worldPos) const
{
	glm::ivec2 result;
	glm::vec4 normalizedTextureCoords = ((glm::inverse(modelTransform) * worldPos)/terrainScale);
	const sf::Vector2u imgSize = materialMap.getSize();
	result[0] = int(std::floor(normalizedTextureCoords.x * imgSize.x));
	result[1] = int(std::floor(normalizedTextureCoords.y * imgSize.y));
	return result;
}

float Terrain::GetMaterialMapPixelSizeInWorldScale() const
{
	assert(materialMap.getSize().x == materialMap.getSize().y);
	return terrainScale / materialMap.getSize().x;
}

void Terrain::DownloadMaterialMapToGPU()
{
	LoadTexture(materialTexture, materialMap, true);
}

void Terrain::SaveMaterialMap() const {
	materialMap.saveToFile(materialMapFilename);
}


////////////////////////////////////////////
//
// Private functions
//
////////////////////////////////////////////

sf::Image Terrain::LoadTexture(gl::Texture& target, const std::string& filename, bool data, float anisotropy)
{
	sf::Image result;

	if (!result.loadFromFile(filename)) {
		throw std::runtime_error((std::string("Can not load texture ") + filename).c_str());
	}

	//srcImg.flipVertically();

	LoadTexture(target, result, data, anisotropy);

	return result;
}

void Terrain::LoadTexture(gl::Texture& target, const sf::Image& srcImg, bool data, float anisotropy)
{
	target.Bind(gl::Texture::Target::_2D);
	gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::LinearMipmapLinear);
	gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);

	if (anisotropy > 0) {
		gl::Texture::Anisotropy(gl::Texture::Target::_2D, anisotropy);
	}

	gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);
	gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);

	gl::Texture::Image2D(gl::Texture::Target::_2D,
		0,
		data ? gl::enums::PixelDataInternalFormat::RGB : gl::enums::PixelDataInternalFormat::SRGB8,
		srcImg.getSize().x,
		srcImg.getSize().y,
		0,
		gl::enums::PixelDataFormat::RGBA,
		gl::enums::DataType::UnsignedByte,
		srcImg.getPixelsPtr());

	gl::Texture::GenerateMipmap(gl::Texture::Target::_2D);
}

glm::vec3 Terrain::GetLowerTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<glm::vec3>& positions, const int numHorizontalVertices)
{
	const int numVerticalVertices = positions.size()/numHorizontalVertices;

	const int x = bottomLeftVertexPosX;
	const int y = bottomLeftVertexPosY;

	if (x + 1 < numHorizontalVertices && y + 1 < numVerticalVertices) {

		const glm::vec3 A = positions.at(GetVertexIndex(numHorizontalVertices, x+0, y+0));
		const glm::vec3 B = positions.at(GetVertexIndex(numHorizontalVertices, x+1, y+0));
		const glm::vec3 C = positions.at(GetVertexIndex(numHorizontalVertices, x+0, y+1));

		return glm::normalize(glm::cross(B-A, C-A));
	}
	else {
		return glm::vec3(0, 0, 0);
	}
}

glm::vec3 Terrain::GetUpperTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<glm::vec3>& positions, const int numHorizontalVertices)
{
	const int numVerticalVertices = positions.size()/numHorizontalVertices;

	const int x = bottomLeftVertexPosX;
	const int y = bottomLeftVertexPosY;

	if (x + 1 < numHorizontalVertices && y + 1 < numVerticalVertices) {

		const glm::vec3 A = positions.at(GetVertexIndex(numHorizontalVertices, x+1, y+1));
		const glm::vec3 B = positions.at(GetVertexIndex(numHorizontalVertices, x+0, y+1));
		const glm::vec3 C = positions.at(GetVertexIndex(numHorizontalVertices, x+1, y+0));

		return glm::normalize(glm::cross(B-A, C-A));
	}
	else {
		return glm::vec3(0, 0, 0);
	}
}

int Terrain::GetVertexIndex(const int width, const int x, const int y)
{
	return y*width + x;
}

std::vector<glm::vec3> Terrain::CalculatePositions(const sf::Image& image, const float scale, const float heightMultiplyer)
{
	static const int maxHeight = 255;
	static const bool fromSRGB = false;

	const size_t imgWidth = image.getSize().x;
	const size_t imgHeight = image.getSize().y;
	const size_t pixelCount = imgWidth * imgHeight;
	const size_t vertexCount = pixelCount;

	std::vector<glm::vec3> tmpPos(vertexCount);

	for (size_t currY = 0; currY < imgHeight; currY++) {
		for (size_t currX = 0; currX < imgWidth; currX++) {
			float currHeight = image.getPixel(currX, currY).r;
			if (fromSRGB) { currHeight = std::pow(currHeight/maxHeight, 2.2f)*maxHeight; }

			tmpPos.at(GetVertexIndex(imgWidth, currX, currY)) = glm::vec3(float(currX)/(imgWidth-1), float(currY)/(imgHeight-1), (float(currHeight)/maxHeight)*heightMultiplyer)*scale;
		}
	}

	std::vector<glm::vec3> result(vertexCount);

	//average heights
	for (size_t currY = 0; currY < imgHeight; currY++) {
		for (size_t currX = 0; currX < imgWidth; currX++) {
			glm::vec2 currPlanePos(tmpPos.at(GetVertexIndex(imgWidth, currX, currY)));
			float currHeight = tmpPos.at(GetVertexIndex(imgWidth, currX, currY)).z;

			float weightedHeightSum = 0;
			float weightSum = 0;

			const float radius = 1;

			for (int dy = int(-radius); dy <= radius+0.01; dy++) {
				for (int dx = int(-radius); dx <= radius+0.01; dx++) {
					const size_t neighbourIdY = currY+dy;
					const size_t neighbourIdX = currX+dx;
					if (neighbourIdX >= 0 && neighbourIdX < imgWidth && neighbourIdY >= 0 && neighbourIdY < imgHeight) {
						glm::vec3 neighbourPos = tmpPos.at(GetVertexIndex(imgWidth, neighbourIdX, neighbourIdY));
						const float neighbourWeight = 1;
						weightSum += neighbourWeight;
						weightedHeightSum += neighbourPos.z*neighbourWeight;
					}
				}
			}

			result.at(GetVertexIndex(imgWidth, currX, currY)) = glm::vec3(currPlanePos, weightedHeightSum / weightSum);
		}
	}

	return result;
}

std::vector<glm::vec2> Terrain::CalculateTexCoords(const sf::Image& image)
{
	const size_t imgWidth = image.getSize().x;
	const size_t imgHeight = image.getSize().y;
	const size_t pixelCount = imgWidth * imgHeight;
	const size_t vertexCount = pixelCount;

	std::vector<glm::vec2> result(vertexCount);

	for (size_t currY = 0; currY < imgHeight; currY++) {
		for (size_t currX = 0; currX < imgWidth; currX++) {
			result.at(GetVertexIndex(imgWidth, currX, currY)) = glm::vec2(float(currX) / (imgWidth - 1), float(currY) / (imgHeight - 1));
		}
	}

	return result;
}

std::vector<glm::vec3> Terrain::CalculateNormals(const sf::Image& image, const std::vector<glm::vec3>& positions)
{
	const size_t imgWidth = image.getSize().x;
	const size_t imgHeight = image.getSize().y;
	const size_t pixelCount = imgWidth * imgHeight;
	const size_t vertexCount = pixelCount;

	std::vector<glm::vec3> result(imgWidth*imgHeight);

	for (size_t currY = 0; currY < imgHeight; currY++) {
		for (size_t currX = 0; currX < imgWidth; currX++) {
			using namespace std;
			const size_t prevX = max(int(currX) - 1, 0);
			const size_t prevY = max(int(currY) - 1, 0);
			std::array<glm::vec3, 6> adjacentTriangleNormals = {
				GetLowerTriangleNormalFromQuad(currX, currY, positions, imgWidth) * 90.f,
				GetUpperTriangleNormalFromQuad(prevX, currY, positions, imgWidth) * 45.f,
				GetLowerTriangleNormalFromQuad(prevX, currY, positions, imgWidth) * 45.f,
				GetUpperTriangleNormalFromQuad(prevX, prevY, positions, imgWidth) * 90.f,
				GetLowerTriangleNormalFromQuad(currX, prevY, positions, imgWidth) * 45.f,
				GetUpperTriangleNormalFromQuad(currX, prevY, positions, imgWidth) * 45.f,
			};

			glm::vec3 normal = glm::vec3(0, 0, 0);
			for (auto& current : adjacentTriangleNormals) {
				normal += current;
			}

			normal = glm::normalize(normal);

			result.at(GetVertexIndex(imgWidth, currX, currY)) = normal;
		}
	}

	return result;
}

std::vector<glm::vec3> Terrain::CalculateTangents(const sf::Image& image, const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& texCoords)
{
	const size_t imgWidth = image.getSize().x;
	const size_t imgHeight = image.getSize().y;
	const size_t pixelCount = imgWidth * imgHeight;
	const size_t vertexCount = pixelCount;

	std::vector<glm::vec3> result(vertexCount);
	//result.reserve(vertexCount);

	for (size_t currY = 0; currY < imgHeight; currY++) {
		for (size_t currX = 0; currX < imgWidth; currX++) {

			glm::vec3 curr = positions.at(GetVertexIndex(imgWidth, currX, currY));
			glm::vec3 adjacent;
			if (currX + 1 < imgWidth) {
				adjacent = positions.at(GetVertexIndex(imgWidth, currX+1, currY));
			}
			else {
				adjacent = positions.at(GetVertexIndex(imgWidth, currX, currY));
				adjacent.x += 1;
			}
			
			result.at(GetVertexIndex(imgWidth, currX, currY)) = glm::normalize(adjacent - curr);
		}
	}

	return result;
}

void Terrain::SetUpIndices(std::vector<Mesh::Submesh::IndexType>* indices, const int imgWidth, const int imgHeight)
{
	int indicesArrayIndex = 0;

	for (int currY = 0; currY < imgHeight; currY++) {
		for (int currX = 0; currX < imgWidth; currX++) {

			if (currX+1 < imgWidth && currY+1 < imgHeight) {
				//"Lower" triangle
				indices->at(indicesArrayIndex++) = GetVertexIndex(imgWidth, currX, currY);
				indices->at(indicesArrayIndex++) = GetVertexIndex(imgWidth, currX+1, currY);
				indices->at(indicesArrayIndex++) = GetVertexIndex(imgWidth, currX, currY+1);

				//"Upper" tringle
				indices->at(indicesArrayIndex++) = GetVertexIndex(imgWidth, currX+1, currY);
				indices->at(indicesArrayIndex++) = GetVertexIndex(imgWidth, currX+1, currY+1);
				indices->at(indicesArrayIndex++) = GetVertexIndex(imgWidth, currX, currY+1);
			}
		}
	}
}
