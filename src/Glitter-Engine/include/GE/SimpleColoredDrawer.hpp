#pragma once

#include "all_gl_headers.hpp"

#include <GE/Mesh.hpp>

class SimpleColoredDrawer
{
public:
	SimpleColoredDrawer();

	void Draw(gl::Context& glContext, const Mesh& mesh, const glm::mat4& MVP, const glm::vec4& color);

private:
	gl::Uniform<glm::vec4> sh_color;
	gl::Uniform<glm::mat4> sh_MVP;

	gl::Program shaderProgram;
};
