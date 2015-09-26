#pragma once

#include "all_gl_headers.hpp"

#include "IFramebuffer.hpp"

class GraphicsEngine;

class Framebuffer : public IFramebuffer
{
public:
	enum AttachmentFlag : uint8_t {
		ATTACHEMNT_COLOR = 1,
		ATTACHEMNT_DEPTH = 2,
		ATTACHEMNT_NORMAL = 4,
		_ATTACHMENT_END_
	};

public:
	Framebuffer();
	Framebuffer(const int width, const int height, uint8_t attachmentFlags = ATTACHEMNT_COLOR | ATTACHEMNT_DEPTH);
	Framebuffer(Framebuffer&&);

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	Framebuffer& operator=(Framebuffer&&);

	const gl::Texture& GetTexture(AttachmentFlag attachmentID) const;

	void Bind(gl::Framebuffer::Target target) const override;
	void Draw(GraphicsEngine& graphicsEngine);

	void SetVertexPosName(const std::string& name);
	void SetColorTexName(const std::string& name);
	void SetDepthTexName(const std::string& name);

	void SetShaderProgram(const gl::Program* program);
	void SetResolution(const int width, const int height);

private:
	static gl::FramebufferAttachment GetGLAttachmentFromFlag(AttachmentFlag flag);

private:
	gl::Framebuffer fbo;
	std::map<uint8_t, gl::Texture> textures;

	gl::VertexArray VAO;
	gl::Buffer vertexPos;
	gl::Buffer indices;

	std::string colorTexName;
	std::string depthTexName;
	std::string vertexPosName;
	const gl::Program *pShaderProgram;
};
