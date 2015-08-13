#pragma once

#include "all_gl_headers.hpp"
#include <array>

using Quad = std::array<gl::Vec3f, 4>;

struct Frustum
{
	Quad nearPlane;
	Quad farPlane;

	Frustum Transformed(const gl::Mat4f& transform);
};

class Camera
{
public:
	virtual ~Camera();

	virtual gl::Mat4f GetProjectionTransform() const = 0;
	virtual gl::Mat4f GetViewTransform() const = 0;
	gl::Mat4f GetViewProjectionTransform() const;

	/**
	 * Get absoulte view frustum corresponding to this camera's current state.
	 */
	Frustum GetFrustum() const;
};
