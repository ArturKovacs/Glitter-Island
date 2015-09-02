#include "SimpleColoredDrawer.hpp"

#include "DemoCore.hpp"

SimpleColoredDrawer::SimpleColoredDrawer()
{
	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("SimpleColored_v.glsl", "SimpleColored_f.glsl");
	shaderProgram.Use();

	sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
	sh_color = gl::Uniform<gl::Vec4f>(shaderProgram, "color");
}

void SimpleColoredDrawer::Draw(gl::Context& glContext, const Mesh& mesh, const gl::Mat4f& MVP, const gl::Vec4f& color)
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
