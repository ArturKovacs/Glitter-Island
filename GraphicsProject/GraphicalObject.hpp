#pragma once

#include "all_gl_headers.hpp"
#include "Mesh.hpp"

//enum class UniformCategory { MVP_MATRIX, MODEL_TRANSPOSED_INVERSE_MATRIX, TEXTURE_DIFFUSE, TEXTURE_NORMAL };

enum class TextureType { COLOR, DATA };

class DemoCore;

class GraphicalObject
{
public:
	GraphicalObject();

	GraphicalObject(GraphicalObject&) = delete;
	GraphicalObject& operator=(GraphicalObject&) = delete;

	GraphicalObject(GraphicalObject&&);
	GraphicalObject& operator=(GraphicalObject&&);

	void SetMesh(Mesh&& newMesh);
	Mesh& GetMesh();

	void Draw(DemoCore& core);

	void SetTransform(const gl::Mat4f& transform);
	gl::Mat4f GetTransform() const;

private:
	//ADD TO MOVE CONSTRUTOR, AND MOVE ASSIGNMENT OPERATOR

	Mesh mesh;

	gl::Texture albedoTexture;
	gl::Texture normalMap;
	gl::Texture specularTexture;
	gl::Texture roughnessTexture;

	gl::Uniform<gl::Vec3f> sh_lightDir;
	gl::Uniform<gl::Mat4f> sh_MVP;
	gl::Uniform<gl::Mat4f> sh_MODELVIEW;
	gl::Uniform<gl::Mat4f> sh_modelTransposedInverse;

	gl::Program shaderProgram;

	gl::Mat4f modelTransform;

private:
	void LoadTexture(gl::Texture& target, const std::string& filename, TextureType type);
};
