#pragma once

#include "all_gl_headers.hpp"
#include "Camera.hpp"
#include "Framebuffer.hpp"

class GraphicsEngine;

class Skybox
{
public:
	Skybox(GraphicsEngine* pGraphicsEngine);

	void LoadTextureFromFiles(
		const std::string& negXfileName,
		const std::string& posXfileName,
		const std::string& negYfileName,
		const std::string& posYfileName,
		const std::string& negZfileName,
		const std::string& posZfileName);

	void Draw();
	
private:
	GraphicsEngine* pGraphEngine;
	
	gl::VertexArray VAO;

	gl::Buffer vertexPositions;
	gl::Buffer indices;

	gl::Texture cubeMap;

	gl::Program skydrawShader;
	gl::Program fadeoutShader;

	gl::Uniform<gl::Mat4f> sh_ViewProjectionMatrix;
	
	Framebuffer skyboxFB;
};

