#pragma once

#include "Mesh.hpp"
#include <SFML/Graphics.hpp>

class DemoCore;

class Terrain
{
private:
	Mesh graphicalModel;
	Mesh seabottom;

	gl::Texture materialTexture;
	gl::Texture sandTexture;
	gl::Texture grassTexture;

	gl::Mat4f modelTransform;
	gl::Uniform<gl::Mat4f> sh_MVP;
	gl::Uniform<gl::Mat4f> sh_modelTransposedInverse;

	gl::Program shaderProgram;
	gl::Uniform<gl::Vec3f> sh_lightDir;

	gl::Vec3f lightDir;

public:
	Terrain();

	void LoadFromHeightMap(const std::string& fileName, float scale, float heightMultiplyer, bool invertNormals = false);
	void Draw(DemoCore& core);

	void SetTransform(const gl::Mat4f& transform);
	gl::Mat4f GetTransform() const;

	void SetLightDir(const gl::Vec3f& vec);

private:
	static void LoadTexture(gl::Texture& target, const std::string& filename, float anisotropy = 0);

	gl::Vec3f GetNormalInHeightMap(const sf::Image& heightMap, const int x, const int y, const float scale, const float heightMultiplyer) const;

	gl::Vec3f GetLowerTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<GLfloat>& positions, const int numHorizontalVertices) const;
	gl::Vec3f GetUpperTriangleNormalFromQuad(const int bottomLeftVertexPosX, const int bottomLeftVertexPosY, const std::vector<GLfloat>& positions, const int numHorizontalVertices) const;

	//gl::Vec3f GetVertexPosition(const std::vector<GLfloat>& positions, const int x, const int y, const int width) const;
	
	void SetAttributeInArray(std::vector<GLfloat>* attributeArray, const int width, const int x, const int y, const gl::Vec3f value) const;
	void SetAttributeInArray(std::vector<GLfloat>* attributeArray, const int width, const int x, const int y, const gl::Vec2f value) const;

	gl::Vec2f GetAttributeFromVec2Array(const std::vector<GLfloat>& attributeArray, const int width, const int x, const int y) const;
	gl::Vec3f GetAttributeFromVec3Array(const std::vector<GLfloat>& attributeArray, const int width, const int x, const int y) const;

	int GetVertexIndex(const int width, const int x, const int y) const;
	int GetVertexIndexInAttributeArray(const int width, const int x, const int y, const int numDimensions) const;
};

