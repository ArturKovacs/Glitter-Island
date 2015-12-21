#pragma once

#include "all_gl_headers.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

class GraphicsEngine;

class GraphicalObject
{
public:
    GraphicalObject();

    void SetMesh(Mesh* newMesh);
    Mesh* GetMesh();

    void Draw(GraphicsEngine* pGraphicsEngine);

    void SetTransform(const glm::mat4& transform);
    glm::mat4 GetTransform() const;
    
    void SetVisible(bool value);
    bool IsVisible() const;
	
	void SetDepthTestEnabled(bool enabled);
	bool IsDepthTestEnabled() const;

private:
    bool visible;
	bool depthTest;
    
    Mesh* pMesh;
    
    glm::mat4 modelTransform;
};
