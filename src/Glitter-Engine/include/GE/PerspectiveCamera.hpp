#pragma once

#include "Camera.hpp"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera();
	~PerspectiveCamera();

	void RotateVertically(const gl::Anglef rot);
	void RotateHorizontally(const gl::Anglef rot);

	/**
	 * amount can be negative!
	 */
	void MoveForward(const float amount);

	/**
	 * amount can be negative!
	 */
	void MoveRight(const float amount);

	void SetHorizontalRot(const gl::Anglef rot);
	void SetVerticalRot(const gl::Anglef rot);
	void SetPosition(const gl::Vec3f& newPos);

	void SetFovY(const gl::Anglef fovy);
	void SetZNear(float near);
	void SetZFar(float far);
	void SetScreenWidth(const int width);
	void SetScreenHeight(const int height);

	gl::Anglef GetHorizontalRot() const;
	gl::Anglef GetVerticalRot() const;
	gl::Vec3f GetPosition() const;

	gl::Anglef GetFovY() const;
	float GetZNear() const;
	float GetZFar() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;

	float GetAspectRatio() const;

	gl::Quatf GetCameraRotation() const;

	gl::Mat4f GetProjectionTransform() const override;
	gl::Mat4f GetViewTransform() const override;

private:
	gl::Anglef horRot;
	gl::Anglef vertRot;
	gl::Vec3f pos;

	gl::Anglef fovy;
	float zNear;
	float zFar;

	int screenWidth;
	int screenHeight;
};
