#pragma once

#include <GE/Mesh.hpp>
#include <GE/Utility.hpp>
#include <map>

class GraphicsEngine;

class MeshManager
{
public:
	util::managed_ptr<Mesh> LoadMeshFromFile(GraphicsEngine* pGraphicsEngine, const std::string& filename);

private:
	std::map<std::string, Mesh*> meshes;

private:
	static Mesh* LoadFromOBJFile(GraphicsEngine* pGraphicsEngine, const std::string& filename);
};
