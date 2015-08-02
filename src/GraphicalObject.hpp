#pragma once

#include "all_gl_headers.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

class DemoCore;

class GraphicalObject
{
public:
	GraphicalObject();

	void SetMesh(Mesh* newMesh);
	Mesh* GetMesh();

	//void SetMaterial(Material* newMaterial);
	//Material* GetMaterial();

	void Draw(DemoCore& core);

	void SetTransform(const gl::Mat4f& transform);
	gl::Mat4f GetTransform() const;

private:
	Mesh* pMesh;
	//Material* pMaterial;

	gl::Mat4f modelTransform;

private:
	//static void LoadTexture(gl::Texture& target, const std::string& filename, TextureType type);
};
