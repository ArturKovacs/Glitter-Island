#include <GE/DebugDrawer.hpp>
#include <GE/GraphicsEngine.hpp>

DebugDrawer::DebugDrawer(GraphicsEngine* pGraphicsEngine) : pGraphicsEngine(pGraphicsEngine), enabled(true), activeCam(nullptr)
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
	if (!enabled) {
		return;
	}

	meshes.push_back(Mesh::GenerateFrustum(frustum));
}

void DebugDrawer::Draw()
{
	if (activeCam == nullptr || !enabled) {
		return;
	}

	pGraphicsEngine->GetGLContext().Clear().ColorBuffer().DepthBuffer();

	for (auto& currMesh : meshes) {
		pGraphicsEngine->GetSimpleColoredDrawer().Draw(pGraphicsEngine->GetGLContext(), currMesh, activeCam->GetViewProjectionTransform(), glm::vec4(1));
	}

	meshes.clear();
}

void DebugDrawer::SetEnabled(bool isEnabled)
{
	enabled = isEnabled;
}
