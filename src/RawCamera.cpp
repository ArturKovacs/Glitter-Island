#include "RawCamera.hpp"

RawCamera::RawCamera()
{}

RawCamera::~RawCamera()
{}

void RawCamera::SetProjectionTransform(const gl::Mat4f& transform)
{
	projectionTransform = transform;
}

void RawCamera::SetViewTransform(const gl::Mat4f& transform)
{
	viewTransform = transform;
}

gl::Mat4f RawCamera::GetProjectionTransform() const
{
	return projectionTransform;
}

gl::Mat4f RawCamera::GetViewTransform() const
{
	return viewTransform;
}
