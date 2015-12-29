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
	gl::Uniform<glm::mat4> sh_MVP;
	gl::Uniform<glm::mat4> sh_invViewProj;
	gl::Uniform<glm::mat4> sh_invMVP;
	gl::UniformSampler sh_screen;
	gl::UniformSampler sh_screenDepth;
	gl::UniformSampler sh_skybox;
	gl::Uniform<GLint> sh_screenWidth;
	gl::Uniform<GLint> sh_screenHeight;
	gl::Uniform<glm::vec3> sh_camPos;
	gl::Uniform<glm::vec3> sh_sunDir;
	gl::Uniform<glm::vec3> sh_sunColor;
	gl::Uniform<GLfloat> sh_time;

	gl::Program geometryOnlyShader;
	gl::Uniform<glm::mat4> sh_geomOnly_MVP;
	
private:
	void Draw();
};

