#pragma once

#include "Camera.hpp"

class OrthoCamera : public Camera
{
public:
	OrthoCamera();
	~OrthoCamera();

	void SetXLeft(float left);
	void SetXRight(float right);
	void SetYBottom(float bottom);
	void SetYTop(float top);
	void SetZNear(float near);
	void SetZFar(float far);

	float GetXLeft();
	float GetXRight();
	float GetYBottom();
	float GetYTop();
	float GetZNear();
	float GetZFar();

	glm::mat4 GetProjectionTransform() const override;
	glm::mat4 GetViewTransform() const override;

private:
	float xLeft, xRight;
	float yBottom, yTop;
	float zNear, zFar;
};
