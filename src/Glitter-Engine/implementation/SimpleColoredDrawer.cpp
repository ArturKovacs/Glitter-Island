#include <GE/SimpleColoredDrawer.hpp>

#include <GE/GraphicsEngine.hpp>

SimpleColoredDrawer::SimpleColoredDrawer()
{
	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("SimpleColored_v.glsl", "SimpleColored_f.glsl");
	shaderProgram.Use();

	sh_MVP = gl::Uniform<glm::mat4>(shaderProgram, "MVP");
	sh_color = gl::Uniform<glm::vec4>(shaderProgram, "color");
}

void SimpleColoredDrawer::Draw(gl::Context& glContext, const Mesh& mesh, const glm::mat4& MVP, const glm::vec4& color)
{
	shaderProgram.Use();

	for (auto& curr : mesh.GetSubmeshes()) {
		curr.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");

		sh_MVP.Set(MVP);
		sh_color.Set(color);

		curr.BindVAO();
		glContext.DrawElements(curr.GetPrimitiveType(), curr.GetNumOfIndices(), curr.indexTypeEnum);
	}
}
