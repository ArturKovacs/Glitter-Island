#pragma once

#include "all_gl_headers.hpp"

class Water
{
	friend class GraphicsEngine;
public:
	Water(const float waterSize = 50);

private:
	gl::VertexArray VAO;

	gl::Buffer vertexPos;
	gl::Buffer indices;

	gl::Program waterShader;
	gl::Uniform<gl::Mat4f> sh_MVP;
	gl::Uniform<gl::Mat4f> sh_viewProj;
	gl::Uniform<gl::Mat4f> sh_invMVP;
	gl::UniformSampler sh_screen;
	gl::UniformSampler sh_screenDepth;
	gl::Uniform<GLint> sh_screenWidth;
	gl::Uniform<GLint> sh_screenHeight;
	gl::Uniform<gl::Vec3f> sh_camPos;
	gl::Uniform<gl::Vec3f> sh_sunDir;
	gl::Uniform<GLfloat> sh_time;

	gl::Program geometryOnlyShader;
	gl::Uniform<gl::Mat4f> sh_geomOnly_MVP;
	
private:
	void Draw(GraphicsEngine& graphicsEngine);
};

