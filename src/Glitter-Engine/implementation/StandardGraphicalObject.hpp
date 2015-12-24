#pragma once

#include <GE/GraphicalObject.hpp>

class Mesh;
class GraphicsEngine;

class StandardGraphicalObject : public GraphicalObject
{
public:
	StandardGraphicalObject();

	virtual void SetMesh(Mesh* newMesh) override;
	virtual Mesh* GetMesh() override;

	virtual void SetTransform(const glm::mat4& transform) override;
	virtual glm::mat4 GetTransform() const override;
	
	virtual void SetVisible(bool value) override;
	virtual bool IsVisible() const override;
	
	virtual void SetDepthTestEnabled(bool enabled) override;
	virtual bool IsDepthTestEnabled() const override;

private:
	bool visible;
	bool depthTest;
	
	Mesh* pMesh;
	
	glm::mat4 modelTransform;
};
