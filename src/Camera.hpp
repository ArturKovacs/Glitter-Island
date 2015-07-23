#pragma once

#include "all_gl_headers.hpp"

class Camera
{
private:
	gl::Anglef horRot;
	gl::Anglef vertRot;
	
	gl::Vec3f pos;

	gl::Anglef fovy;

	int screenWidth;
	int screenHeight;

public:

	Camera();

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
	void SetScreenWidth(const int width);
	void SetScreenHeight(const int height);

	gl::Anglef GetHorizontalRot() const;
	gl::Anglef GetVerticalRot() const;
	gl::Vec3f GetPosition() const;

	gl::Anglef GetFovY() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;

	float GetAspectRatio() const;

	gl::Quatf GetCameraRotation() const;

	gl::Mat4f GetProjectionTransform() const;
	gl::Mat4f GetViewTransform() const;
	gl::Mat4f GetViewProjectionTransform() const;
};

