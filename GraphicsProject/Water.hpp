#pragma once

#include "all_gl_headers.hpp"

class DemoCore;

class Water
{
private:
	gl::VertexArray VAO;

	gl::Buffer vertexPos;
	gl::Buffer indices;

	gl::Program waterShader;
	gl::Uniform<gl::Mat4f> sh_MVP;
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

public:
	Water(const float waterSize = 50);

	void Draw(DemoCore& core);
	void DrawGeometryOnly(DemoCore& core);

private:
	//void DrawBehindWaterWithRefraction(DemoCore& core, Framebuffer& screenWithoutWater);
	//void DrawInFrontOfWater(DemoCore& core, Framebuffer& screenWithoutWater);
};

