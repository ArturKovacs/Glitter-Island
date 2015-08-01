#pragma once

#include "all_gl_headers.hpp"

class Mesh;

class Material
{
public:
	virtual ~Material();

	virtual void Prepare(Mesh* pMesh, gl::Mat4f& modelTransform) = 0;
};

