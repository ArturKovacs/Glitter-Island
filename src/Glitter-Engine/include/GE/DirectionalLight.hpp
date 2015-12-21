#pragma once

#include "all_gl_headers.hpp"

class DirectionalLight
{
private:
	glm::vec3 directionTowardsSource;
	glm::vec3 color;

public:
	DirectionalLight(glm::vec3 directionTowardsSource = glm::vec3(1, 0, 0), glm::vec3 color = glm::vec3(1, 1, 1));

	void SetDirectionTowardsSource(const glm::vec3& directionTowardsSource);
	void SetColor(const glm::vec3& color);

	glm::vec3 GetDirectionTowardsSource() const;
	glm::vec3 GetColor() const;
};

