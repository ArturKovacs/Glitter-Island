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

void GraphicalObject::Draw(DemoCore& core)
{
	if (pMesh == nullptr) {
		return;
	}

	auto& glContext = core.GetGLContext();
	
	glContext.Disable(gl::Capability::CullFace);
	glContext.Enable(gl::Capability::Blend);
	gl::Context::BlendFunc(gl::enums::BlendFunction::SrcAlpha, gl::enums::BlendFunction::OneMinusSrcAlpha);

	for (auto& curr : pMesh->GetSubmeshes()) {
		Material* pMaterial = curr.GetMaterial();
		if (pMaterial != nullptr) {
			pMaterial->Prepare(curr, modelTransform);
			curr.BindVAO();
			glContext.DrawElements(curr.GetPrimitiveType(), curr.GetNumOfIndices(), curr.indexTypeEnum);
		}
	}

	glContext.Disable(gl::Capability::Blend);
	glContext.Enable(gl::Capability::CullFace);
}

void GraphicalObject::DrawDepthOnly(DemoCore& core, DepthOnlyMaterial& depthMaterial)
{
	if (pMesh == nullptr) {
		return;
	}

	auto& glContext = core.GetGLContext();
	
	glContext.Disable(gl::Capability::CullFace);
	glContext.Enable(gl::Capability::Blend);
	gl::Context::BlendFunc(gl::enums::BlendFunction::SrcAlpha, gl::enums::BlendFunction::OneMinusSrcAlpha);

	for (auto& curr : pMesh->GetSubmeshes()) {
		Material* pMaterial = curr.GetMaterial();
		if (pMaterial != nullptr) {
			depthMaterial.SetTextureContainingAlpha(pMaterial->GetTextureContainigAlpha());
			depthMaterial.Prepare(curr, modelTransform);
			curr.BindVAO();
			glContext.DrawElements(curr.GetPrimitiveType(), curr.GetNumOfIndices(), curr.indexTypeEnum);
		}
	}

	glContext.Disable(gl::Capability::Blend);
	glContext.Enable(gl::Capability::CullFace);
}

void GraphicalObject::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f GraphicalObject::GetTransform() const
{
	return modelTransform;
}
