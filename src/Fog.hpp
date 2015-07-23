#pragma once

#include "all_gl_headers.hpp"

class DemoCore;

class Fog
{
private:

	gl::Program shaderProgram;

public:
	Fog();

	void Draw(DemoCore& core);
};

