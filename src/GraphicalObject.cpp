#include "GraphicalObject.hpp"

#include "DemoCore.hpp"

GraphicalObject::GraphicalObject() : /*pMaterial(nullptr),*/ pMesh(nullptr)
{
	visible = true;
	depthTest = true;
}

void GraphicalObject::SetMesh(Mesh* newMesh)
{
	pMesh = newMesh;
}

Mesh* GraphicalObject::GetMesh()
{
	return pMesh;
}

void GraphicalObject::Draw(GraphicsEngine* pGraphicsEngine)
{
	if (pMesh == nullptr || !visible) {
		return;
	}

	auto& glContext = pGraphicsEngine->GetGLContext();
	
	glContext.Disable(gl::Capability::CullFace);
	glContext.Enable(gl::Capability::Blend);
	gl::Context::BlendFunc(gl::enums::BlendFunction::SrcAlpha, gl::enums::BlendFunction::OneMinusSrcAlpha);

	if (depthTest) {
		glContext.Enable(gl::Capability::DepthTest);
	}
	else {
		glContext.Disable(gl::Capability::DepthTest);
	}
	
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

void GraphicalObject::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f GraphicalObject::GetTransform() const
{
	return modelTransform;
}

void GraphicalObject::SetVisible(bool value)
{
	visible = value;
}

bool GraphicalObject::IsVisible() const
{
	return visible;
}

void GraphicalObject::SetDepthTestEnabled(bool enabled)
{
	depthTest = enabled;
}

bool GraphicalObject::IsDepthTestEnabled() const
{
	return depthTest;
}

