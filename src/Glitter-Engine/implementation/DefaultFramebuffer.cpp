#include <GE/DefaultFramebuffer.hpp>

void DefaultFramebuffer::Bind(gl::Framebuffer::Target target) const
{
	dfbo.Bind(target);
}
