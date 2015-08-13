#pragma once

#include "Camera.hpp"

class RawCamera : public Camera
{
public:
	RawCamera();
	~RawCamera();

	void SetProjectionTransform(const gl::Mat4f& transform);
	void SetViewTransform(const gl::Mat4f& transform);

	gl::Mat4f GetProjectionTransform() const override;
	gl::Mat4f GetViewTransform() const override;

private:
	gl::Mat4f projectionTransform;
	gl::Mat4f viewTransform;
};
