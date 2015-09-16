#include <GE/PerspectiveCamera.hpp>

PerspectiveCamera::PerspectiveCamera() :
horRot(gl::Radians(0)),
vertRot(gl::Radians(0)),
pos(0, 0, 0),
fovy(gl::Degrees(60)),
zNear(0.5),
zFar(500),
screenWidth(0),
screenHeight(0)
{}

PerspectiveCamera::~PerspectiveCamera()
{}

void PerspectiveCamera::RotateVertically(const gl::Anglef rot)
{
	vertRot += rot;
}

void PerspectiveCamera::RotateHorizontally(const gl::Anglef rot)
{
	horRot += rot;
}

void PerspectiveCamera::MoveForward(const float amount)
{
	const gl::Vec3f forward(0, 0, -1);
	pos += gl::Quatf::RotateVector(GetCameraRotation(), forward * amount);
}

void PerspectiveCamera::MoveRight(const float amount)
{
	const gl::Vec3f right(1, 0, 0);
	pos += gl::Quatf::RotateVector(GetCameraRotation(), right * amount);
}

void PerspectiveCamera::SetHorizontalRot(const gl::Anglef rotRad)
{
	horRot = rotRad;
}

void PerspectiveCamera::SetVerticalRot(const gl::Anglef rotRad)
{
	vertRot = gl::Degrees(std::min(std::max(rotRad.ValueInDegrees(), -90.f), 90.f));
}

void PerspectiveCamera::SetPosition(const gl::Vec3f& newPos)
{
	pos = newPos;
}

void PerspectiveCamera::SetFovY(const gl::Anglef fovy)
{
	this->fovy = fovy;
}

void PerspectiveCamera::SetZNear(float near)
{
	zNear = near;
}

void PerspectiveCamera::SetZFar(float far)
{
	zFar = far;
}

void PerspectiveCamera::SetScreenWidth(const int width)
{
	screenWidth = width;
}

void PerspectiveCamera::SetScreenHeight(const int height)
{
	screenHeight = height;
}

gl::Anglef PerspectiveCamera::GetHorizontalRot() const
{
	return horRot;
}

gl::Anglef PerspectiveCamera::GetVerticalRot() const
{
	return vertRot;
}

gl::Vec3f PerspectiveCamera::GetPosition() const
{
	return pos;
}

gl::Anglef PerspectiveCamera::GetFovY() const
{
	return fovy;
}

float PerspectiveCamera::GetZNear() const
{
	return zNear;
}

float PerspectiveCamera::GetZFar() const
{
	return zFar;
}

int PerspectiveCamera::GetScreenWidth() const
{
	return screenWidth;
}

int PerspectiveCamera::GetScreenHeight() const
{
	return screenHeight;
}

float PerspectiveCamera::GetAspectRatio() const
{
	return float(screenWidth)/screenHeight;
}

gl::Quatf PerspectiveCamera::GetCameraRotation() const
{
	return gl::Quatf(gl::Vec3f(0, 1, 0), horRot) * gl::Quatf(gl::Vec3f(1, 0, 0), vertRot);
}

gl::Mat4f PerspectiveCamera::GetProjectionTransform() const
{
	return gl::CamMatrixf::PerspectiveY(fovy, GetAspectRatio(), zNear, zFar);
}

gl::Mat4f PerspectiveCamera::GetViewTransform() const
{
	return gl::ModelMatrixf::RotationQ(gl::Quatf::Inverse(GetCameraRotation())) * gl::ModelMatrixf::Translation(-pos);
}
