#pragma once

#include "all_gl_headers.hpp"

#include "Mesh.hpp"
#include "Material.hpp"
#include <SFML/Graphics.hpp>

class GraphicsEngine;

class Terrain
{
public:
	Terrain(GraphicsEngine* pGraphicsEngine);

	void LoadFromHeightMap(const std::string& fileName, float scale, float heightMultiplyer, bool invertNormals = false);
	void Draw();

	void SetTransform(const glm::mat4& transform);
	glm::mat4 GetTransform() const;

	sf::Image& GetMaterialMap();
	glm::ivec2 GetMaterialMapPos(const glm::vec4 worldPos) const;
	float GetMaterialMapPixelSizeInWorldScale() const;
	void DownloadMaterialMapToGPU();
	void SaveMaterialMap() const;

private:
	GraphicsEngine* pGraphicsEngine;

	Mesh terrainModel;
	Mesh seabottom;

	float terrainScale;
	std::string materialMapFilename;
	sf::Image materialMap;

	gl::Texture materialTexture;
	gl::Texture sandTexture;
	gl::Texture sandNormalMap;
	gl::Texture grassTexture;
	gl::Texture grassNormalMap;


	glm::mat4 modelTransform;
	std::vector<gl::Uniform<glm::mat4>> sh_worldToShadowMap;
	std::vector<gl::Uniform<float>> sh_viewSubfrustumFarPlanesTexDepth;

	gl::Program shaderProgram;
	gl::Uniform<glm::mat4> sh_modelTransform;
	gl::Uniform<glm::mat4> sh_modelViewTransform;
	gl::Uniform<glm::mat4> sh_MVP;
	gl::Uniform<glm::mat4> sh_modelTrInv;
	gl::Uniform<glm::vec3> sh_sunDir;
	gl::Uniform<glm::vec3> sh_sunColor;

	gl::Program seabottomProgram;
	gl::Uniform<glm::mat4> seabottom_MVP;
	gl::Uniform<glm::vec4> seabottom_color;

private:
	static sf::Image LoadTexture(gl::Texture& target, const std::string& filename, bool data, float anisotropy = 0);
	static void LoadTexture(gl::Texture& target, const sf::Image& srcImg, bool data, float anisotropy = 0);

	static glm::vec3 GetLowerTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<glm::vec3>& positions, const int numHorizontalVertices);
	static glm::vec3 GetUpperTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<glm::vec3>& positions, const int numHorizontalVertices);

	static int GetVertexIndex(const int width, const int x, const int y);

	static std::vector<glm::vec3> CalculatePositions(const sf::Image& image, const float scale, const float heightMultiplyer);
	static std::vector<glm::vec2> CalculateTexCoords(const sf::Image& image);
	static std::vector<glm::vec3> CalculateNormals(const sf::Image& image, const std::vector<glm::vec3>& positions);
	static std::vector<glm::vec3> CalculateTangents(const sf::Image& image, const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& texCoords);

	static void SetUpIndices(std::vector<Mesh::Submesh::IndexType>* indices, const int imgWidth, const int imgHeight);
	
};

