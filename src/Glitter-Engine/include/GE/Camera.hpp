#pragma once

#include "all_gl_headers.hpp"
#include <array>

using Quad = std::array<glm::vec3, 4>;

struct Frustum
{
	Quad nearPlane;
	Quad farPlane;

	Frustum Transformed(const glm::mat4& transform);
};

class Camera
{
public:
	virtual ~Camera();

	virtual glm::mat4 GetProjectionTransform() const = 0;
	virtual glm::mat4 GetViewTransform() const = 0;
	glm::mat4 GetViewProjectionTransform() const;

	/**
	 * Get absoulte view frustum corresponding to this camera's current state.
	 */
	Frustum GetFrustum() const;
};
