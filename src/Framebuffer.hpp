#pragma once

#include "all_gl_headers.hpp"

#include "IFramebuffer.hpp"

class DemoCore;

class Framebuffer : public IFramebuffer
{
public:
	Framebuffer(const int width, const int height);
	Framebuffer(Framebuffer&&);

	Framebuffer(const Framebuffer&) = delete;
	void operator=(const Framebuffer&) = delete;

	const gl::Texture& GetColorTexture() const;
	const gl::Texture& GetDepthTexture() const;

	void Bind(gl::Framebuffer::Target target) const override;
	void Draw(DemoCore& core);

	void SetVertexPosName(const std::string& name);
	void SetColorTexName(const std::string& name);
	void SetDepthTexName(const std::string& name);

	void SetShaderProgram(const gl::Program* program);
	void SetResolution(const int width, const int height);

private:
	gl::Framebuffer fbo;
	gl::Texture colorTex;
	gl::Texture depthTex;

	gl::VertexArray VAO;
	gl::Buffer vertexPos;
	gl::Buffer indices;

	std::string colorTexName;
	std::string depthTexName;
	std::string vertexPosName;
	const gl::Program *pShaderProgram;
};
