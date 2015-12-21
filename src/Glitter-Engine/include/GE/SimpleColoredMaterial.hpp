#pragma once

#include "Material.hpp"
#include "all_gl_headers.hpp"

class GraphicsEngine;

class SimpleColoredMaterial : public Material
{
public:
    SimpleColoredMaterial(GraphicsEngine* pGraphicsEngine);
    ~SimpleColoredMaterial();
    
    void Prepare(Mesh::Submesh& submsh, const glm::mat4& modelTransform) override;
	void Prepare(Mesh::Submesh& submsh) override;
	void SetTransform(const glm::mat4& modelTransform) override;
    
    glm::vec4 GetColor() const;
    void SetColor(const glm::vec4& newColor);
    
private:
    GraphicsEngine* pGraphicsEngine;
    
    glm::vec4 color;
            
    gl::Uniform<glm::vec4> sh_color;
    gl::Uniform<glm::mat4> sh_MVP;

    gl::Program shaderProgram;
};
