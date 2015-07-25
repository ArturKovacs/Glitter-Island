#pragma once

#include "all_gl_headers.hpp"

#include "Mesh.hpp"
#include <SFML/Graphics.hpp>

class DemoCore;

class Terrain
{
public:
	Terrain();

	void LoadFromHeightMap(const std::string& fileName, float scale, float heightMultiplyer, bool invertNormals = false);
	void Draw(DemoCore& core);

	void SetTransform(const gl::Mat4f& transform);
	gl::Mat4f GetTransform() const;

	void SetLightDir(const gl::Vec3f& vec);

	sf::Image& GetMaterialMap();
	gl::Vec2i GetMaterialMapPos(const gl::Vec4f worldPos) const;
	void UpdateMaterialMap();

private:
	Mesh graphicalModel;
	Mesh seabottom;

	float terrainScale;
	sf::Image materialMap;

	gl::Texture materialTexture;
	gl::Texture sandTexture;
	gl::Texture grassTexture;

	gl::Mat4f modelTransform;
	gl::Uniform<gl::Mat4f> sh_MVP;
	gl::Uniform<gl::Mat4f> sh_modelTransposedInverse;

	gl::Program shaderProgram;
	gl::Uniform<gl::Vec3f> sh_lightDir;

	gl::Vec3f lightDir;

private:
	static void LoadTexture(gl::Texture& target, sf::Image& srcImg, const std::string& filename, float anisotropy = 0);
	static void LoadTexture(gl::Texture& target, const sf::Image& srcImg, float anisotropy = 0);

	static gl::Vec3f GetLowerTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<gl::Vec3f>& positions, const int numHorizontalVertices);
	static gl::Vec3f GetUpperTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<gl::Vec3f>& positions, const int numHorizontalVertices);

	static int GetVertexIndex(const int width, const int x, const int y);

	static void CalculatePositionsAndTexCoords(std::vector<gl::Vec3f>* positions, std::vector<gl::Vec2f>* texCoords, const sf::Image& image, const float scale, const float heightMultiplyer);
	static void CalculateNormals(std::vector<gl::Vec3f>* normals, const sf::Image& image, const std::vector<gl::Vec3f>& positions);

	static void SetUpIndices(std::vector<Mesh::IndexType>* indices, const int imgWidth, const int imgHeight);
};
