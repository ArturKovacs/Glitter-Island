#include "SimpleColoredMaterial.hpp"

#include "GraphicsEngine.hpp"

SimpleColoredMaterial::SimpleColoredMaterial(GraphicsEngine* pGraphicsEngine) :
pGraphicsEngine(pGraphicsEngine),
color(0, 0, 0, 0)
{}

SimpleColoredMaterial::~SimpleColoredMaterial()
{}

gl::Vec4f SimpleColoredMaterial::GetColor() const
{
	return color;
}

void SimpleColoredMaterial::SetColor(const gl::Vec4f& newColor)
{
	color = newColor;
}

void SimpleColoredMaterial::Prepare(Mesh::Submesh& submsh, gl::Mat4f& modelTransform)
{
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");

	sh_MVP.Set(pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
	sh_color.Set(color);
}
