#pragma once

#include "all_gl_headers.hpp"

#include "Mesh.hpp"

class SimpleColoredDrawer
{
public:
	SimpleColoredDrawer();

	void Draw(gl::Context& glContext, const Mesh& mesh, const gl::Mat4f& MVP, const gl::Vec4f& color);

private:
	gl::Uniform<gl::Vec4f> sh_color;
	gl::Uniform<gl::Mat4f> sh_MVP;

	gl::Program shaderProgram;
};

