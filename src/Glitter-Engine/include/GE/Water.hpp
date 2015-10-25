#pragma once

#include "all_gl_headers.hpp"
#include "Framebuffer.hpp"

class GraphicsEngine;

class Water
{
	friend class GraphicsEngine;
public:
	Water(GraphicsEngine* pGraphicsEngine, const float waterSize = 50);
	
private:
	GraphicsEngine* pGraphEngine;
	Framebuffer targetFB;
	
	bool visible;
	
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
	void Draw();
};

