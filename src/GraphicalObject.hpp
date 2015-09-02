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

    void SetTransform(const gl::Mat4f& transform);
    gl::Mat4f GetTransform() const;
    
    void SetVisible(bool value);
    bool IsVisible() const;
	
	void SetDepthEnabled(bool enabled);
	bool IsDepthTestEnabled() const;

private:
    bool visible;
	bool depthTest;
    
    Mesh* pMesh;
    
    gl::Mat4f modelTransform;
};
