#pragma once

#include "all_gl_headers.hpp"
#include "Mesh.hpp"

class Material
{
public:
	virtual ~Material();

	virtual void Prepare(Mesh::Submesh& submsh, const gl::Mat4f& modelTransform) = 0;
	virtual void Prepare(Mesh::Submesh& submsh) = 0;
	virtual void SetTransform(const gl::Mat4f& modelTransform) = 0;
};
