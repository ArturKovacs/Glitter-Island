#include "DirectionalLight.hpp"


DirectionalLight::DirectionalLight(gl::Vec3f directionTowardsSource, gl::Vec3f color) :
	directionTowardsSource(directionTowardsSource), color(color)
{
}

void DirectionalLight::SetDirectionTowardsSource(const gl::Vec3f& directionTowardsSource)
{
	this->directionTowardsSource = gl::Normalized(directionTowardsSource);
}

void DirectionalLight::SetColor(const gl::Vec3f& color)
{
	this->color = color;
}

gl::Vec3f DirectionalLight::GetDirectionTowardsSource() const
{
	return directionTowardsSource;
}

gl::Vec3f DirectionalLight::GetColor() const
{
	return color;
}
