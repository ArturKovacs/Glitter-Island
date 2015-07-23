#include "Camera.hpp"

#include <cmath>

Camera::Camera() : horRot(gl::Radians(0)), vertRot(gl::Radians(0)), pos(0, 0, 0), screenWidth(0), screenHeight(0) {}

void Camera::RotateVertically(const gl::Anglef rot)
{
	vertRot += rot;
}

void Camera::RotateHorizontally(const gl::Anglef rot)
{
	horRot += rot;
}

void Camera::MoveForward(const float amount)
{
	const gl::Vec3f forward(0, 0, -1);
	pos += gl::Quatf::RotateVector(GetCameraRotation(), forward * amount);
}

void Camera::MoveRight(const float amount)
{
	const gl::Vec3f right(1, 0, 0);
	pos += gl::Quatf::RotateVector(GetCameraRotation(), right * amount);
}

void Camera::SetHorizontalRot(const gl::Anglef rotRad)
{
	horRot = rotRad;
}

void Camera::SetVerticalRot(const gl::Anglef rotRad)
{
	vertRot = gl::Degrees(std::min(std::max(rotRad.ValueInDegrees(), -90.f), 90.f));
}

void Camera::SetPosition(const gl::Vec3f& newPos)
{
	pos = newPos;
}

void Camera::SetFovY(const gl::Anglef fovy)
{
	this->fovy = fovy;
}

void Camera::SetScreenWidth(const int width)
{
	screenWidth = width;
}

void Camera::SetScreenHeight(const int height)
{
	screenHeight = height;
}

gl::Anglef Camera::GetHorizontalRot() const
{
	return horRot;
}

gl::Anglef Camera::GetVerticalRot() const
{
	return vertRot;
}

gl::Vec3f Camera::GetPosition() const
{
	return pos;
}

gl::Anglef Camera::GetFovY() const
{
	return fovy;
}

int Camera::GetScreenWidth() const
{
	return screenWidth;
}

int Camera::GetScreenHeight() const
{
	return screenHeight;
}

float Camera::GetAspectRatio() const
{
	return float(screenWidth)/screenHeight;
}

gl::Quatf Camera::GetCameraRotation() const
{
	return gl::Quatf(gl::Vec3f(0, 1, 0), horRot) * gl::Quatf(gl::Vec3f(1, 0, 0), vertRot);
}

gl::Mat4f Camera::GetProjectionTransform() const
{
	return gl::CamMatrixf::PerspectiveY(fovy, GetAspectRatio(), 0.5f, 400.f);
}

gl::Mat4f Camera::GetViewTransform() const
{
	return gl::ModelMatrixf::RotationQ(gl::Quatf::Inverse(GetCameraRotation())) * gl::ModelMatrixf::Translation(-pos);
}

gl::Mat4f Camera::GetViewProjectionTransform() const
{
	return GetProjectionTransform() * GetViewTransform();
}
