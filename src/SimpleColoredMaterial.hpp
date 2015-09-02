#pragma once

#include "Material.hpp"
#include "all_gl_headers.hpp"

class GraphicsEngine;

class SimpleColoredMaterial : public Material
{
public:
    SimpleColoredMaterial(GraphicsEngine* pGraphicsEngine);
    ~SimpleColoredMaterial();
    
    void Prepare(Mesh::Submesh& submsh, gl::Mat4f& modelTransform);
    
    gl::Vec4f GetColor() const;
    void SetColor(const gl::Vec4f& newColor);
    
private:
    GraphicsEngine* pGraphicsEngine;
    
    gl::Vec4f color;
            
    gl::Uniform<gl::Vec4f> sh_color;
    gl::Uniform<gl::Mat4f> sh_MVP;

    gl::Program shaderProgram;
};