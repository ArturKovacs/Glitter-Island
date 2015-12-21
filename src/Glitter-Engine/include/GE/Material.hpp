#pragma once

#include "all_gl_headers.hpp"
#include "Mesh.hpp"

class Material
{
public:
	virtual ~Material();

	virtual void Prepare(Mesh::Submesh& submsh, const glm::mat4& modelTransform) = 0;
	virtual void Prepare(Mesh::Submesh& submsh) = 0;
	virtual void SetTransform(const glm::mat4& modelTransform) = 0;
};
