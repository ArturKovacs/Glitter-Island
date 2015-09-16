#include <GE/Camera.hpp>

#include <initializer_list>
#include <cmath>

Frustum Frustum::Transformed(const gl::Mat4f& transform)
{
	Frustum result;
	
	for (int i = 0; i < nearPlane.size(); i++) {
		auto transformed = transform * gl::Vec4f(nearPlane[i], 1);
		result.nearPlane[i] = (transformed / transformed.w()).xyz();
	}
	
	for (int i = 0; i < farPlane.size(); i++) {
		auto transformed = transform * gl::Vec4f(farPlane[i], 1);
		result.farPlane[i] = (transformed / transformed.w()).xyz();
	}

	return std::move(result);
}

Camera::~Camera()
{}

gl::Mat4f Camera::GetViewProjectionTransform() const
{
	return GetProjectionTransform() * GetViewTransform();
}

Frustum Camera::GetFrustum() const
{
	Frustum result;

	result.nearPlane[0] = gl::Vec3f(-1, -1, -1);
	result.nearPlane[1] = gl::Vec3f(+1, -1, -1);
	result.nearPlane[2] = gl::Vec3f(+1, +1, -1);
	result.nearPlane[3] = gl::Vec3f(-1, +1, -1);

	result.farPlane[0] = gl::Vec3f(-1, -1, +1);
	result.farPlane[1] = gl::Vec3f(+1, -1, +1);
	result.farPlane[2] = gl::Vec3f(+1, +1, +1);
	result.farPlane[3] = gl::Vec3f(-1, +1, +1);

	gl::Mat4f inverseCameraTransform = gl::Inverse(GetViewProjectionTransform());

	result = result.Transformed(inverseCameraTransform);

	return std::move(result);
}
