#pragma once

#include "all_gl_headers.hpp"

class DirectionalLight
{
private:
	gl::Vec3f directionTowardsSource;
	gl::Vec3f color;

public:
	DirectionalLight(gl::Vec3f directionTowardsSource = gl::Vec3f(1, 0, 0), gl::Vec3f color = gl::Vec3f(1, 1, 1));

	void SetDirectionTowardsSource(const gl::Vec3f& directionTowardsSource);
	void SetColor(const gl::Vec3f& color);

	gl::Vec3f GetDirectionTowardsSource() const;
	gl::Vec3f GetColor() const;
};

