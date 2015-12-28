#pragma once

#include "all_gl_headers.hpp"

#include "IFramebuffer.hpp"

class GraphicsEngine;

class Framebuffer : public IFramebuffer
{
public:
	enum AttachmentFlag : uint8_t {
		ATTACHMENT_COLOR = 1,
		ATTACHMENT_DEPTH = 2,
		ATTACHMENT_NORMAL = 4,
		ATTACHMENT_VIEW_DEPTH = 8,
		_ATTACHMENT_END_
	};
	
private:
	struct TextureShaderID {
		std::string name;
		int index;
		
		TextureShaderID() = default;
		TextureShaderID(std::string name, int index) : name(name), index(index) {}
	};

public:
	Framebuffer();
	Framebuffer(const int width, const int height, uint8_t attachmentFlags = ATTACHMENT_COLOR | ATTACHMENT_DEPTH);
	Framebuffer(Framebuffer&&) = default;
	Framebuffer& operator=(Framebuffer&&) = default;

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	
	void CopyFramebufferContents(const Framebuffer& source);

	const gl::Texture& GetTexture(AttachmentFlag attachmentID) const;

	void Bind(gl::Framebuffer::Target target) const override;
	void Draw(GraphicsEngine& graphicsEngine);

	void SetVertexPosName(const std::string& name);
	void SetTextureShaderID(AttachmentFlag attachmentID, std::string name, int index);

	void SetShaderProgram(const gl::Program* program, uint8_t usedAttachmentFlags);
	void SetResolution(const int width, const int height);

private:
	static gl::FramebufferAttachment GetGLAttachmentFromFlag(AttachmentFlag flag);

private:
	gl::Framebuffer fbo;
	std::map<uint8_t, gl::Texture> textures;
	uint8_t usedAttachments;

	gl::VertexArray VAO;
	gl::Buffer vertexPos;
	gl::Buffer indices;

	std::map<uint8_t, TextureShaderID> textureShaderIDs;
	//std::string colorTexName;
	//std::string depthTexName;
	std::string vertexPosName;
	const gl::Program *pShaderProgram;
	
	int w, h;
};
