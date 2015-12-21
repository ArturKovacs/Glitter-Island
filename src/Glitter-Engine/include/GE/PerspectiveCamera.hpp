#pragma once

#include "Camera.hpp"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera();
	~PerspectiveCamera();

	void RotateVertically(const float rot);
	void RotateHorizontally(const float rot);

	/**
	 * amount can be negative!
	 */
	void MoveForward(const float amount);

	/**
	 * amount can be negative!
	 */
	void MoveRight(const float amount);

	void SetHorizontalRot(const float rot);
	void SetVerticalRot(const float rot);
	void SetPosition(const glm::vec3& newPos);

	void SetFovY(const float fovy);
	void SetZNear(float near);
	void SetZFar(float far);
	void SetScreenWidth(const int width);
	void SetScreenHeight(const int height);

	float GetHorizontalRot() const;
	float GetVerticalRot() const;
	glm::vec3 GetPosition() const;

	float GetFovY() const;
	float GetZNear() const;
	float GetZFar() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;

	float GetAspectRatio() const;

	glm::quat GetCameraRotation() const;

	glm::mat4 GetProjectionTransform() const override;
	glm::mat4 GetViewTransform() const override;

private:
	float horRot;
	float vertRot;
	glm::vec3 pos;

	float fovy;
	float zNear;
	float zFar;

	int screenWidth;
	int screenHeight;
};
