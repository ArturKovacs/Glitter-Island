#include "DebugDrawer.hpp"
#include "DemoCore.hpp"

DebugDrawer::DebugDrawer(DemoCore* pCore) : pCore(pCore), activeCam(nullptr)
{}

void DebugDrawer::SetActiveCam(Camera* cam)
{
	activeCam = cam;
}

const Camera* DebugDrawer::GetActiveCam() const
{
	return activeCam;
}

void DebugDrawer::DrawOnce(const Frustum& frustum)
{
	meshes.push_back(Mesh::GenerateFrustum(frustum));
}

void DebugDrawer::Draw()
{
	if (activeCam == nullptr) {
		return;
	}

	for (auto& currMesh : meshes) {
		pCore->simpleColoredDrawer.Draw(pCore->GetGLContext(), currMesh, activeCam->GetViewProjectionTransform(), gl::Vec4f(1));
	}

	meshes.clear();
}
