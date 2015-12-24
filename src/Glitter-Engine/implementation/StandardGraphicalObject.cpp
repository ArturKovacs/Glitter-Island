#include "StandardGraphicalObject.hpp"

#include <GE/GraphicsEngine.hpp>

StandardGraphicalObject::StandardGraphicalObject() : /*pMaterial(nullptr),*/ pMesh(nullptr)
{
	visible = true;
	depthTest = true;
}

void StandardGraphicalObject::SetMesh(Mesh* newMesh)
{
	pMesh = newMesh;
}

Mesh* StandardGraphicalObject::GetMesh()
{
	return pMesh;
}

void StandardGraphicalObject::SetTransform(const glm::mat4& transform)
{
	modelTransform = transform;
}

glm::mat4 StandardGraphicalObject::GetTransform() const
{
	return modelTransform;
}

void StandardGraphicalObject::SetVisible(bool value)
{
	visible = value;
}

bool StandardGraphicalObject::IsVisible() const
{
	return visible;
}

void StandardGraphicalObject::SetDepthTestEnabled(bool enabled)
{
	depthTest = enabled;
}

bool StandardGraphicalObject::IsDepthTestEnabled() const
{
	return depthTest;
}

