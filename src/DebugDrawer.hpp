#pragma once

#include "PerspectiveCamera.hpp"
#include "Mesh.hpp"
#include <vector>

class DemoCore;

class DebugDrawer
{
public:
	DebugDrawer(DemoCore* pCore);

	void SetActiveCam(Camera* cam);
	const Camera* GetActiveCam() const;

	void DrawOnce(const Frustum& frustum);

	void Draw();

	void SetEnabled(bool isEnabled);

private:
	DemoCore* pCore;

	bool enabled;
	Camera* activeCam;
	std::vector<Mesh> meshes;
};

