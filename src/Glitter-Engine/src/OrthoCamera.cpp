#include <GE/OrthoCamera.hpp>

#include <glm/gtc/matrix_transform.hpp>

OrthoCamera::OrthoCamera() :
xLeft(-1), xRight(1),
yBottom(-1), yTop(1),
zNear(-1), zFar(1)
{
	//This class is not done.
	assert(false);
}

OrthoCamera::~OrthoCamera()
{}

void OrthoCamera::SetXLeft(float left)
{
	xLeft = left;
}

void OrthoCamera::SetXRight(float right)
{
	xRight = right;
}

void OrthoCamera::SetYBottom(float bottom)
{
	yBottom = bottom;
}

void OrthoCamera::SetYTop(float top)
{
	yTop = top;
}

void OrthoCamera::SetZNear(float near)
{
	zNear = near;
}

void OrthoCamera::SetZFar(float far)
{
	zFar = far;
}

float OrthoCamera::GetXLeft()
{
	return xLeft;
}

float OrthoCamera::GetXRight()
{
	return xRight;
}

float OrthoCamera::GetYBottom()
{
	return yBottom;
}

float OrthoCamera::GetYTop()
{
	return yTop;
}

float OrthoCamera::GetZNear()
{
	return zNear;
}

float OrthoCamera::GetZFar()
{
	return zFar;
}

glm::mat4 OrthoCamera::GetProjectionTransform() const
{
	return glm::ortho(xLeft, xRight, yBottom, yTop, zNear, zFar);
}

glm::mat4 OrthoCamera::GetViewTransform() const
{
	return glm::mat4();
}
