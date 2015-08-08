#pragma once

#include "Mesh.hpp"
#include <map>

class DemoCore;

class MeshManager
{
public:
	Mesh* LoadMeshFromFile(DemoCore* pCore, const std::string& filename);

private:
	std::map<std::string, Mesh*> meshes;

private:
	Mesh* LoadFromOBJFile(DemoCore* pCore, const std::string& filename);
};
