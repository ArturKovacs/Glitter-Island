#pragma once

#include "all_gl_headers.hpp"
#include "Mesh.hpp"

class Material
{
public:
	virtual ~Material();

	//virtual const gl::Texture* GetTextureContainigAlpha() const = 0;
	virtual void Prepare(Mesh::Submesh& submsh, gl::Mat4f& modelTransform) = 0;
};
