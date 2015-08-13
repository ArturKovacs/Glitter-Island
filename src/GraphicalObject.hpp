#pragma once

#include "all_gl_headers.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "DepthOnlyMaterial.hpp"

class DemoCore;

class GraphicalObject
{
public:
	GraphicalObject();

	void SetMesh(Mesh* newMesh);
	Mesh* GetMesh();

	void Draw(DemoCore& core);
	void DrawDepthOnly(DemoCore& core, DepthOnlyMaterial& depthMaterial);

	void SetTransform(const gl::Mat4f& transform);
	gl::Mat4f GetTransform() const;

private:
	Mesh* pMesh;

	gl::Mat4f modelTransform;
};
