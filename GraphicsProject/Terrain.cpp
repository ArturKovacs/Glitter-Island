#include "Terrain.hpp"

#include "all_gl_headers.hpp"
#include "DemoCore.hpp"

#include <oglplus/images/load.hpp>
#include "FileLoad.hpp"

#include <SFML/Graphics.hpp>
#include <exception>

Terrain::Terrain()
{
	shaderProgram = DemoCore::LoadShaderProgramFromFiles("terrain_v.glsl", "terrain_f.glsl");
	shaderProgram.Use();

	try {
		sh_lightDir = gl::Uniform<gl::Vec3f>(shaderProgram, "lightDir");
		sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
		sh_modelTransposedInverse = gl::Uniform<gl::Mat4f>(shaderProgram, "model_transposed_inverse");
		gl::UniformSampler(shaderProgram, "sandTexture").Set(0);
		gl::UniformSampler(shaderProgram, "grassTexture").Set(1);
		gl::UniformSampler(shaderProgram, "materialTexture").Set(2);
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	LoadTexture(sandTexture, DemoCore::imgFolderPath + "sand_seamless.png", 4);
	LoadTexture(grassTexture, DemoCore::imgFolderPath + "grass_seamless.png", 4);
	LoadTexture(materialTexture, DemoCore::imgFolderPath + "materialMap.png");
}

void Terrain::LoadFromHeightMap(const std::string& fileName, float scale, float heightMultiplyer, bool invertNormals)
{
	static const int maxHeight = 255;
	static const bool fromSRGB = false;

	//gl::images::Image oglplusimg = gl::images::LoadByName("", fileName, true, true);
	//oglplusimg.Pixel(0, 0, oglplusimg.Depth()).x;

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

	//const int coordinateDimensions = 3;
	//const int texCoordDimensions = 2;

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

	std::vector<Mesh::IndexType> indices(pixelCountWithAssociatedQuad * trianglesInAQuad * triangleVertexCount);
	SetUpIndices(&indices, imgWidth, imgHeight);

	shaderProgram.Use();

	graphicalModel.SetVertexAttributeBuffer(AttributeCategory::POSITION, positions);
	graphicalModel.SetVertexAttributeBuffer(AttributeCategory::NORMAL, normals);
	graphicalModel.SetVertexAttributeBuffer(AttributeCategory::TEX_COORD, texCoords);

	graphicalModel.SetIndices(indices);

	try {
		graphicalModel.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
		graphicalModel.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
		graphicalModel.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	graphicalModel.SetPrimitiveType(gl::enums::PrimitiveType::Triangles);

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

	std::vector<Mesh::IndexType> seaBottomIndices = {
		0, 1, 2, 3
	};

	seabottom.SetVertexAttributeBuffer(AttributeCategory::POSITION, seaBottomVertPos);
	seabottom.SetVertexAttributeBuffer(AttributeCategory::NORMAL, seaBottomVertNormal);

	seabottom.SetIndices(seaBottomIndices);

	try {
		seabottom.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
		seabottom.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	seabottom.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);
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

void Terrain::Draw(DemoCore& core)
{
	shaderProgram.Use();
	gl::Texture::Active(0);
	sandTexture.Bind(gl::Texture::Target::_2D);

	gl::Texture::Active(1);
	grassTexture.Bind(gl::Texture::Target::_2D);

	gl::Texture::Active(2);
	materialTexture.Bind(gl::Texture::Target::_2D);

	sh_lightDir.Set(lightDir);
	sh_MVP.Set(core.GetCamera().GetViewProjectionTransform() * modelTransform);
	sh_modelTransposedInverse.Set(SajatTransposeMertNemMukodikAzOglPlusOsTODO(gl::Inverse(modelTransform)));
	//sh_modelTransposedInverse.Set(gl::Transposed(gl::Inverse(modelTransform)));

	graphicalModel.BindVAO();
	core.GetGLContext().DrawElements(graphicalModel.GetPrimitiveType(), graphicalModel.GetNumOfIndices(), graphicalModel.indexTypeEnum);

	seabottom.BindVAO();
	core.GetGLContext().DrawElements(seabottom.GetPrimitiveType(), seabottom.GetNumOfIndices(), seabottom.indexTypeEnum);
}

void Terrain::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f Terrain::GetTransform() const
{
	return modelTransform;
}

void Terrain::SetLightDir(const gl::Vec3f& vec)
{
	lightDir = vec;
}

void Terrain::LoadTexture(gl::Texture& target, const std::string& filename, float anisotropy)
{
	sf::Image texture;

	if (!texture.loadFromFile(filename)) {
		throw std::exception((std::string("Can not load texture ") + filename).c_str());
	}

	target.Bind(gl::Texture::Target::_2D);
	gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::Linear);
	gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);

	if (anisotropy > 0) {
		gl::Texture::Anisotropy(gl::Texture::Target::_2D, anisotropy);
	}

	gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);
	gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);

	gl::Texture::Image2D(gl::Texture::Target::_2D,
		0,
		gl::enums::PixelDataInternalFormat::SRGB8,
		texture.getSize().x,
		texture.getSize().y,
		0,
		gl::enums::PixelDataFormat::RGBA,
		gl::enums::DataType::UnsignedByte,
		texture.getPixelsPtr());
}

/*
gl::Vec3f Terrain::GetNormalInHeightMap(const sf::Image& heightMap, const int x, const int y, const float scale, const float heightMultiplyer)
{
	const float maxHeight = 255;

	float currHeight = (heightMap.getPixel(x, y).r/maxHeight) * heightMultiplyer;

	float heightBeforeX = currHeight;
	float heightAfterX = currHeight;
	float heightBeforeY = currHeight;
	float heightAfterY = currHeight;

	if (x-1 >= 0) {
		heightBeforeX = (heightMap.getPixel(x-1, y).r/maxHeight) * heightMultiplyer;
	}
	if (x+1 < heightMap.getSize().x) {
		heightAfterX = (heightMap.getPixel(x+1, y).r/maxHeight) * heightMultiplyer;
	}
	if (y-1 >= 0) {
		heightBeforeY = (heightMap.getPixel(x, y-1).r/maxHeight) * heightMultiplyer;
	}
	if (y+1 < heightMap.getSize().y) {
		heightAfterY = (heightMap.getPixel(x, y+1).r/maxHeight) * heightMultiplyer;
	}

	gl::Vec3f slopeX = gl::Vec3f(2 * (1.f/heightMap.getSize().x), 0, heightAfterX - heightBeforeX);
	gl::Vec3f slopeY = gl::Vec3f(0, 2 * (1.f/heightMap.getSize().y), heightAfterY - heightBeforeY);

	return gl::Normalized(gl::Cross(slopeX, slopeY));
}
*/

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

			//gl::Vec3f normal = GetNormalInHeightMap(image, currX, currY, scale, heightMultiplyer);
			//if (invertNormals) { normal = -1.f * normal; }

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
						//const gl::Vec2f posDiff = currPlanePos-neighbourPos.xy();
						//const float planeDistSqr = gl::Vec2f::DotProduct(posDiff, posDiff);
						//const float neighbourWeight = std::exp(-planeDistSqr);
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

void Terrain::SetUpIndices(std::vector<Mesh::IndexType>* indices, const int imgWidth, const int imgHeight)
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
