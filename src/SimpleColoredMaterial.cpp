#include "SimpleColoredMaterial.hpp"

#include "GraphicsEngine.hpp"

SimpleColoredMaterial::SimpleColoredMaterial(GraphicsEngine* pGraphicsEngine) :
pGraphicsEngine(pGraphicsEngine),
color(0, 0, 0, 1)
{
	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("SimpleColored_v.glsl", "SimpleColored_f.glsl");
	shaderProgram.Use();

	sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
	sh_color = gl::Uniform<gl::Vec4f>(shaderProgram, "color");
}

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
	shaderProgram.Use();
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");

	sh_MVP.Set(pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
	sh_color.Set(color);
}
