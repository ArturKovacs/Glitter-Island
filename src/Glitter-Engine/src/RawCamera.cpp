#include <GE/RawCamera.hpp>

RawCamera::RawCamera()
{}

RawCamera::~RawCamera()
{}

void RawCamera::SetProjectionTransform(const glm::mat4& transform)
{
	projectionTransform = transform;
}

void RawCamera::SetViewTransform(const glm::mat4& transform)
{
	viewTransform = transform;
}

glm::mat4 RawCamera::GetProjectionTransform() const
{
	return projectionTransform;
}

glm::mat4 RawCamera::GetViewTransform() const
{
	return viewTransform;
}
