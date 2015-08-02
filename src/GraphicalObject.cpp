#include "GraphicalObject.hpp"

#include "DemoCore.hpp"

GraphicalObject::GraphicalObject() : /*pMaterial(nullptr),*/ pMesh(nullptr)
{
}

void GraphicalObject::SetMesh(Mesh* newMesh)
{
	pMesh = newMesh;
}

Mesh* GraphicalObject::GetMesh()
{
	return pMesh;
}

/*
void GraphicalObject::SetMaterial(Material* newMaterial)
{
	pMaterial = newMaterial;
}

Material* GraphicalObject::GetMaterial()
{
	return pMaterial;
}
*/

void GraphicalObject::Draw(DemoCore& core)
{
	if (/*pMaterial == nullptr ||*/ pMesh == nullptr) {
		return;
	}

	auto& glContext = core.GetGLContext();
	
	for (auto& curr : pMesh->GetSubmeshes()) {
		Material* pMaterial = curr.GetMaterial();
		if (pMaterial != nullptr) {
			pMaterial->Prepare(curr, modelTransform);
			curr.BindVAO();
			glContext.DrawElements(curr.GetPrimitiveType(), curr.GetNumOfIndices(), curr.indexTypeEnum);
		}
	}
}

void GraphicalObject::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f GraphicalObject::GetTransform() const
{
	return modelTransform;
}
