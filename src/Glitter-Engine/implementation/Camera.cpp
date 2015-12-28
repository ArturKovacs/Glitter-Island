#include <GE/Camera.hpp>

#include <initializer_list>
#include <cmath>

Frustum Frustum::Transformed(const glm::mat4& transform)
{
	Frustum result;
	
	for (size_t i = 0; i < nearPlane.size(); i++) {
		auto transformed = transform * glm::vec4(nearPlane[i], 1);
		result.nearPlane[i] = glm::vec3(transformed / transformed.w);
	}
	
	for (size_t i = 0; i < farPlane.size(); i++) {
		auto transformed = transform * glm::vec4(farPlane[i], 1);
		result.farPlane[i] = glm::vec3(transformed / transformed.w);
	}

	return std::move(result);
}

Camera::~Camera()
{}

glm::mat4 Camera::GetViewProjectionTransform() const
{
	return GetProjectionTransform() * GetViewTransform();
}

Frustum Camera::GetFrustum() const
{
	Frustum result;

	result.nearPlane[0] = glm::vec3(-1, -1, -1);
	result.nearPlane[1] = glm::vec3(+1, -1, -1);
	result.nearPlane[2] = glm::vec3(+1, +1, -1);
	result.nearPlane[3] = glm::vec3(-1, +1, -1);

	result.farPlane[0] = glm::vec3(-1, -1, +1);
	result.farPlane[1] = glm::vec3(+1, -1, +1);
	result.farPlane[2] = glm::vec3(+1, +1, +1);
	result.farPlane[3] = glm::vec3(-1, +1, +1);

	glm::mat4 inverseCameraTransform = glm::inverse(GetViewProjectionTransform());

	result = result.Transformed(inverseCameraTransform);

	return std::move(result);
}
