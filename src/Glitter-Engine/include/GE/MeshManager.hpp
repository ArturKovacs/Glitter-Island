#pragma once

#include "Mesh.hpp"
#include <map>

class GraphicsEngine;

class MeshManager
{
public:
	Mesh* LoadMeshFromFile(GraphicsEngine* pGraphicsEngine, const std::string& filename);

private:
	std::map<std::string, Mesh*> meshes;

private:
	Mesh* LoadFromOBJFile(GraphicsEngine* pGraphicsEngine, const std::string& filename);
};
