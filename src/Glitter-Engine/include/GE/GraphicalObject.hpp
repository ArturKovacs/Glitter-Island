#pragma once

#include <glm/glm.hpp>

class Mesh;

class GraphicalObject
{
public:
	virtual void SetMesh(Mesh* newMesh) = 0;
	virtual Mesh* GetMesh() = 0;

	virtual void SetTransform(const glm::mat4& transform) = 0;
	virtual glm::mat4 GetTransform() const = 0;

	virtual void SetVisible(bool value) = 0;
	virtual bool IsVisible() const = 0;

	virtual void SetDepthTestEnabled(bool enabled) = 0;
	virtual bool IsDepthTestEnabled() const = 0;
};
