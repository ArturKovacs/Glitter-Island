#include <GE/SimpleColoredMaterial.hpp>

#include <GE/GraphicsEngine.hpp>

SimpleColoredMaterial::SimpleColoredMaterial(GraphicsEngine* pGraphicsEngine) :
pGraphicsEngine(pGraphicsEngine),
color(0, 0, 0, 1)
{
	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("SimpleColored_v.glsl", "SimpleColored_f.glsl");
	shaderProgram.Use();

	sh_MVP = gl::Uniform<glm::mat4>(shaderProgram, "MVP");
	sh_color = gl::Uniform<glm::vec4>(shaderProgram, "color");
}

SimpleColoredMaterial::~SimpleColoredMaterial()
{}

glm::vec4 SimpleColoredMaterial::GetColor() const
{
	return color;
}

void SimpleColoredMaterial::SetColor(const glm::vec4& newColor)
{
	color = newColor;
}

void SimpleColoredMaterial::Prepare(Mesh::Submesh& submsh, const glm::mat4& modelTransform)
{
	Prepare(submsh);
	SetTransform(modelTransform);
}

void SimpleColoredMaterial::Prepare(Mesh::Submesh& submsh)
{
	shaderProgram.Use();
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");

	sh_color.Set(color);
}

void SimpleColoredMaterial::SetTransform(const glm::mat4& modelTransform)
{
	shaderProgram.Use();
	sh_MVP.Set(pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
}
