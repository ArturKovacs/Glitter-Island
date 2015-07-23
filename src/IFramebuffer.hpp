#pragma once

#include "all_gl_headers.hpp"

class IFramebuffer
{
public:
	virtual ~IFramebuffer();

	virtual void Bind(gl::Framebuffer::Target target) const = 0;
};

