#include <GE/PerspectiveCamera.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>

PerspectiveCamera::PerspectiveCamera() :
horRot(0),
vertRot(0),
pos(0, 0, 0),
fovy(glm::radians(60.f)),
zNear(0.5),
zFar(500),
screenWidth(0),
screenHeight(0)
{}

PerspectiveCamera::~PerspectiveCamera()
{}

void PerspectiveCamera::RotateVertically(const float rot)
{
	vertRot += rot;
}

void PerspectiveCamera::RotateHorizontally(const float rot)
{
	horRot += rot;
}

void PerspectiveCamera::MoveForward(const float amount)
{
	const glm::vec3 forward(0, 0, -1);
	pos += glm::rotate(GetCameraRotation(), forward * amount);
}

void PerspectiveCamera::MoveRight(const float amount)
{
	const glm::vec3 right(1, 0, 0);
	pos += glm::rotate(GetCameraRotation(), right * amount);
}

void PerspectiveCamera::SetHorizontalRot(const float rotRad)
{
	horRot = rotRad;
}

void PerspectiveCamera::SetVerticalRot(const float rotRad)
{
	vertRot = std::min(std::max(rotRad, -glm::pi<float>()), glm::pi<float>());
}

void PerspectiveCamera::SetPosition(const glm::vec3& newPos)
{
	pos = newPos;
}

void PerspectiveCamera::SetFovY(const float fovy)
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

float PerspectiveCamera::GetHorizontalRot() const
{
	return horRot;
}

float PerspectiveCamera::GetVerticalRot() const
{
	return vertRot;
}

glm::vec3 PerspectiveCamera::GetPosition() const
{
	return pos;
}

float PerspectiveCamera::GetFovY() const
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

glm::quat PerspectiveCamera::GetCameraRotation() const
{
	return glm::angleAxis(horRot, glm::vec3(0, 1, 0)) * glm::angleAxis(vertRot, glm::vec3(1, 0, 0));
}

glm::mat4 PerspectiveCamera::GetProjectionTransform() const
{
	return glm::perspective(fovy, GetAspectRatio(), zNear, zFar);
}

glm::mat4 PerspectiveCamera::GetViewTransform() const
{
	return glm::mat4(glm::inverse(GetCameraRotation())) * glm::translate(glm::mat4(1.f), -pos);
}
