#pragma once

#include "all_gl_headers.hpp"

#include "IFramebuffer.hpp"

class DefaultFramebuffer : public IFramebuffer
{
private:
	gl::DefaultFramebuffer dfbo;

public:
	DefaultFramebuffer();

	void Bind(gl::Framebuffer::Target target) const override;
};

