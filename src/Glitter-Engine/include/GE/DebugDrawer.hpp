#pragma once

#include "PerspectiveCamera.hpp"
#include "Mesh.hpp"
#include <vector>

class GraphicsEngine;

class DebugDrawer
{
public:
	DebugDrawer(GraphicsEngine* pGraphicsEngine);

	void SetActiveCam(Camera* cam);
	const Camera* GetActiveCam() const;

	void DrawOnce(const Frustum& frustum);

	void Draw();

	void SetEnabled(bool isEnabled);

private:
	GraphicsEngine* pGraphicsEngine;

	bool enabled;
	Camera* activeCam;
	std::vector<Mesh> meshes;
};

