#pragma once

#include "Camera.hpp"

class RawCamera : public Camera
{
public:
	RawCamera();
	~RawCamera();

	void SetProjectionTransform(const glm::mat4& transform);
	void SetViewTransform(const glm::mat4& transform);

	glm::mat4 GetProjectionTransform() const override;
	glm::mat4 GetViewTransform() const override;

private:
	glm::mat4 projectionTransform;
	glm::mat4 viewTransform;
};
