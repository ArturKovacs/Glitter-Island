#include <GE/Terrain.hpp>

#include <GE/GraphicsEngine.hpp>

#include <SFML/Graphics.hpp>
#include <exception>
#include <cmath>
#include <string>

#define IGNORE_TRY(x) try{x;}catch(std::exception&ex){std::cout<<ex.what()<<std::endl;}

Terrain::Terrain(GraphicsEngine* pGraphicsEngine) : pGraphicsEngine(pGraphicsEngine)
{
	terrainScale = 0;

	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("Terrain_v.glsl", "Terrain_f.glsl");
	shaderProgram.Use();

	try {
		sh_sunDir = gl::Uniform<gl::Vec3f>(shaderProgram, "sunDir");
		sh_sunColor = gl::Uniform<gl::Vec3f>(shaderProgram, "sunColor");
		sh_modelTransform = gl::Uniform<gl::Mat4f>(shaderProgram, "modelTransform");
		sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
		sh_modelTransposedInverse = gl::Uniform<gl::Mat4f>(shaderProgram, "model_transposed_inverse");
		gl::UniformSampler(shaderProgram, "sandTexture").Set(0);
		gl::UniformSampler(shaderProgram, "grassTexture").Set(1);
		gl::UniformSampler(shaderProgram, "materialTexture").Set(2);

		sh_worldToShadowMap.resize(pGraphicsEngine->GetLightCascadeCount());
		sh_viewSubfrustumFarPlanesTexDepth.resize(pGraphicsEngine->GetLightCascadeCount());

		int currTextureID = 3;
		for (int i = 0; i < pGraphicsEngine->GetLightCascadeCount(); i++) {
			IGNORE_TRY(gl::UniformSampler(shaderProgram, "cascadeShadowMaps[" + std::to_string(i) + "]").Set(currTextureID++));
			IGNORE_TRY(sh_worldToShadowMap.at(i) = gl::Uniform<gl::Mat4f>(shaderProgram, "worldToShadowMap[" + std::to_string(i) + "]"));
			IGNORE_TRY(sh_viewSubfrustumFarPlanesTexDepth.at(i) = gl::Uniform<float>(shaderProgram, "viewSubfrustumFarPlanesTexDepth[" + std::to_string(i) + "]"));
		}
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	materialMapFilename = GraphicsEngine::GetImgFolderPath() + "materialMap.png";

	sf::Image tmpImg;
	LoadTexture(sandTexture, tmpImg, GraphicsEngine::GetImgFolderPath() + "sand_seamless.png", false, 4);
	LoadTexture(grassTexture, tmpImg, GraphicsEngine::GetImgFolderPath() + "grass_seamless.png", false, 4);
	LoadTexture(materialTexture, materialMap, materialMapFilename, true);
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

	const int imgWidth = image.getSize().x;
	const int imgHeight = image.getSize().y;

	if (imgWidth < 2 || imgHeight < 2) {
		throw std::runtime_error("Terrain heightmap must be at least 2 pixel big in each direction!");
	}

	const int pixelCount = imgWidth * imgHeight;
	const int totalVertexCount = pixelCount;

	std::vector<gl::Vec3f> positions(totalVertexCount);
	std::vector<gl::Vec2f> texCoords(totalVertexCount);
	std::vector<gl::Vec3f> normals(totalVertexCount);
	CalculatePositionsAndTexCoords(&positions, &texCoords, image, scale, heightMultiplyer);
	CalculateNormals(&normals, image, positions);

	const int pixelCountWithAssociatedQuad = (pixelCount-imgWidth-imgHeight)+1;
	const int trianglesInAQuad = 2;
	const int triangleVertexCount = 3;

	std::vector<Mesh::Submesh::IndexType> indices(pixelCountWithAssociatedQuad * trianglesInAQuad * triangleVertexCount);
	SetUpIndices(&indices, imgWidth, imgHeight);

	shaderProgram.Use();

	Mesh::Submesh submsh;

	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, positions);
	submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, normals);
	submsh.SetVertexAttributeBuffer(AttributeCategory::TEX_COORD, texCoords);

	submsh.SetIndices(indices);

	try {
		submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
		submsh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
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

	Mesh::Submesh seabottom_submsh;

	seabottom_submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, seaBottomVertPos);
	seabottom_submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, seaBottomVertNormal);

	seabottom_submsh.SetIndices(seaBottomIndices);

	try {
		seabottom_submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
		seabottom_submsh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	seabottom_submsh.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);

	seabottom.GetSubmeshes().push_back(std::move(seabottom_submsh));
}

static gl::Mat4f MyTranspose(const gl::Mat4f& input)
{
	gl::Mat4f result;

	for (std::size_t i = 0; i != 4; ++i) {
		for (std::size_t j = 0; j != 4; ++j) {
			result.Set(i, j, input.At(j, i));
		}
	}

	return result;
}

void Terrain::Draw()
{
	shaderProgram.Use();
	gl::Texture::Active(0);
	sandTexture.Bind(gl::Texture::Target::_2D);

	gl::Texture::Active(1);
	grassTexture.Bind(gl::Texture::Target::_2D);

	gl::Texture::Active(2);
	materialTexture.Bind(gl::Texture::Target::_2D);

	int currTextureID = 3;
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
	sh_MVP.Set(pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
	sh_modelTransposedInverse.Set(MyTranspose(gl::Inverse(modelTransform)));
	//sh_modelTransposedInverse.Set(gl::Transposed(gl::Inverse(modelTransform)));

	pGraphicsEngine->GetGLContext().Enable(gl::Capability::CullFace);
	pGraphicsEngine->GetGLContext().Enable(gl::Capability::DepthTest);
	Mesh::Submesh& terrain_submsh = terrainModel.GetSubmeshes().at(0);
	terrain_submsh.BindVAO();
	pGraphicsEngine->GetGLContext().DrawElements(terrain_submsh.GetPrimitiveType(), terrain_submsh.GetNumOfIndices(), terrain_submsh.indexTypeEnum);

	Mesh::Submesh& seabottom_submsh = seabottom.GetSubmeshes().at(0);
	seabottom_submsh.BindVAO();
	pGraphicsEngine->GetGLContext().DrawElements(seabottom_submsh.GetPrimitiveType(), seabottom_submsh.GetNumOfIndices(), seabottom_submsh.indexTypeEnum);
}

void Terrain::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f Terrain::GetTransform() const
{
	return modelTransform;
}

sf::Image& Terrain::GetMaterialMap()
{
	return materialMap;
}

gl::Vec2i Terrain::GetMaterialMapPos(const gl::Vec4f worldPos) const
{
	gl::Vec2i result;
	gl::Vec4f normalizedTextureCoords = ((gl::Inverse(modelTransform) * worldPos)/terrainScale);
	const sf::Vector2u imgSize = materialMap.getSize();
	result[0] = std::floor(normalizedTextureCoords.x() * imgSize.x);
	result[1] = std::floor(normalizedTextureCoords.y() * imgSize.y);
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

void Terrain::LoadTexture(gl::Texture& target, sf::Image& srcImg, const std::string& filename, bool data, float anisotropy)
{
	if (!srcImg.loadFromFile(filename)) {
		throw std::runtime_error((std::string("Can not load texture ") + filename).c_str());
	}

	//srcImg.flipVertically();

	LoadTexture(target, srcImg, data, anisotropy);
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

gl::Vec3f Terrain::GetLowerTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<gl::Vec3f>& positions, const int numHorizontalVertices)
{
	const int numVerticalVertices = positions.size()/numHorizontalVertices;

	const int x = bottomLeftVertexPosX;
	const int y = bottomLeftVertexPosY;

	if (x + 1 < numHorizontalVertices && y + 1 < numVerticalVertices) {

		const gl::Vec3f A = positions.at(GetVertexIndex(numHorizontalVertices, x+0, y+0));
		const gl::Vec3f B = positions.at(GetVertexIndex(numHorizontalVertices, x+1, y+0));
		const gl::Vec3f C = positions.at(GetVertexIndex(numHorizontalVertices, x+0, y+1));

		return gl::Normalized(gl::Cross(B-A, C-A));
	}
	else {
		return gl::Vec3f(0, 0, 0);
	}
}

gl::Vec3f Terrain::GetUpperTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<gl::Vec3f>& positions, const int numHorizontalVertices)
{
	const int numVerticalVertices = positions.size()/numHorizontalVertices;

	const int x = bottomLeftVertexPosX;
	const int y = bottomLeftVertexPosY;

	if (x + 1 < numHorizontalVertices && y + 1 < numVerticalVertices) {

		const gl::Vec3f A = positions.at(GetVertexIndex(numHorizontalVertices, x+1, y+1));
		const gl::Vec3f B = positions.at(GetVertexIndex(numHorizontalVertices, x+0, y+1));
		const gl::Vec3f C = positions.at(GetVertexIndex(numHorizontalVertices, x+1, y+0));

		return gl::Normalized(gl::Cross(B-A, C-A));
	}
	else {
		return gl::Vec3f(0, 0, 0);
	}
}

int Terrain::GetVertexIndex(const int width, const int x, const int y)
{
	return y*width + x;
}

void Terrain::CalculatePositionsAndTexCoords(std::vector<gl::Vec3f>* positions, std::vector<gl::Vec2f>* texCoords, const sf::Image& image, const float scale, const float heightMultiplyer)
{
	static const int maxHeight = 255;
	static const bool fromSRGB = false;

	const int imgWidth = image.getSize().x;
	const int imgHeight = image.getSize().y;

	for (int currY = 0; currY < imgHeight; currY++) {
		for (int currX = 0; currX < imgWidth; currX++) {
			float currHeight = image.getPixel(currX, currY).r;
			if (fromSRGB) { currHeight = std::pow(currHeight/maxHeight, 2.2)*maxHeight; }

			positions->at(GetVertexIndex(imgWidth, currX, currY)) = gl::Vec3f(float(currX)/(imgWidth-1), float(currY)/(imgHeight-1), (float(currHeight)/maxHeight)*heightMultiplyer)*scale;
			texCoords->at(GetVertexIndex(imgWidth, currX, currY)) = gl::Vec2f(float(currX)/(imgWidth-1), float(currY)/(imgHeight-1));
		}
	}

	std::vector<gl::Vec3f> finalPositions(positions->size());

	//average heights
	for (int currY = 0; currY < imgHeight; currY++) {
		for (int currX = 0; currX < imgWidth; currX++) {
			gl::Vec2f currPlanePos = positions->at(GetVertexIndex(imgWidth, currX, currY)).xy();
			float currHeight = positions->at(GetVertexIndex(imgWidth, currX, currY)).z();

			float weightedHeightSum = 0;
			float weightSum = 0;

			const float radius = 1;

			for (int dy = -radius; dy <= radius; dy++) {
				for (int dx = -radius; dx <= radius; dx++) {
					const int neighbourIdY = currY+dy;
					const int neighbourIdX = currX+dx;
					if (neighbourIdX >= 0 && neighbourIdX < imgWidth && neighbourIdY >= 0 && neighbourIdY < imgHeight) {
						gl::Vec3f neighbourPos = positions->at(GetVertexIndex(imgWidth, neighbourIdX, neighbourIdY));
						const float neighbourWeight = 1;
						weightSum += neighbourWeight;
						weightedHeightSum += neighbourPos.z()*neighbourWeight;
					}
				}
			}

			finalPositions.at(GetVertexIndex(imgWidth, currX, currY)) = gl::Vec3f(currPlanePos, weightedHeightSum / weightSum);
		}
	}

	*positions = std::move(finalPositions);
}

void Terrain::CalculateNormals(std::vector<gl::Vec3f>* normals, const sf::Image& image, const std::vector<gl::Vec3f>& positions)
{
	const int imgWidth = image.getSize().x;
	const int imgHeight = image.getSize().y;

	for (int currY = 0; currY < imgHeight; currY++) {
		for (int currX = 0; currX < imgWidth; currX++) {
			std::array<gl::Vec3f, 6> adjacentTriangleNormals = {
				GetLowerTriangleNormalFromQuad(currX, currY, positions, imgWidth) * 90,
				(currX - 1 >= 0 ? GetUpperTriangleNormalFromQuad(currX - 1, currY, positions, imgWidth) : gl::Vec3f(0, 0, 0))*45,
				(currX - 1 >= 0 ? GetLowerTriangleNormalFromQuad(currX - 1, currY, positions, imgWidth) : gl::Vec3f(0, 0, 0))*45,
				(currX - 1 >= 0 && currY - 1 >= 0 ? GetUpperTriangleNormalFromQuad(currX - 1, currY - 1, positions, imgWidth) : gl::Vec3f(0, 0, 0))*90,
				(currY - 1 >= 0 ? GetLowerTriangleNormalFromQuad(currX, currY - 1, positions, imgWidth) : gl::Vec3f(0, 0, 0))*45,
				(currY - 1 >= 0 ? GetUpperTriangleNormalFromQuad(currX, currY - 1, positions, imgWidth) : gl::Vec3f(0, 0, 0))*45,
			};

			gl::Vec3f normal = gl::Vec3f(0, 0, 0);
			for (auto& current : adjacentTriangleNormals) {
				normal += current;
			}

			normal = gl::Normalized(normal);

			normals->at(GetVertexIndex(imgWidth, currX, currY)) = normal;
		}
	}
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
