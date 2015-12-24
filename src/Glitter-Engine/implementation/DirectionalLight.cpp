#include <GE/DirectionalLight.hpp>


DirectionalLight::DirectionalLight(glm::vec3 directionTowardsSource, glm::vec3 color) :
	directionTowardsSource(directionTowardsSource), color(color)
{
}

void DirectionalLight::SetDirectionTowardsSource(const glm::vec3& directionTowardsSource)
{
	this->directionTowardsSource = glm::normalize(directionTowardsSource);
}

void DirectionalLight::SetColor(const glm::vec3& color)
{
	this->color = color;
}

glm::vec3 DirectionalLight::GetDirectionTowardsSource() const
{
	return directionTowardsSource;
}

glm::vec3 DirectionalLight::GetColor() const
{
	return color;
}
