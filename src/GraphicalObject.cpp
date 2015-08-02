#include "GraphicalObject.hpp"

#include "DemoCore.hpp"

GraphicalObject::GraphicalObject() : pMaterial(nullptr), pMesh(nullptr)
{


	//LoadTexture(albedoTexture, DemoCore::imgFolderPath + "Cerberus_A.tga", TextureType::COLOR);
	//LoadTexture(normalMap, DemoCore::imgFolderPath + "Cerberus_N.tga", TextureType::DATA);
	//LoadTexture(specularTexture, DemoCore::imgFolderPath + "Cerberus_M.tga", TextureType::DATA);
	//LoadTexture(roughnessTexture, DemoCore::imgFolderPath + "Cerberus_R.tga", TextureType::DATA);
}

void GraphicalObject::SetMesh(Mesh* newMesh)
{
	pMesh = newMesh;
}

Mesh* GraphicalObject::GetMesh()
{
	return pMesh;
}

void GraphicalObject::SetMaterial(Material* newMaterial)
{
	pMaterial = newMaterial;
}

Material* GraphicalObject::GetMaterial()
{
	return pMaterial;
}

void GraphicalObject::Draw(DemoCore& core)
{
	if (pMaterial == nullptr || pMesh == nullptr) {
		return;
	}

	auto& glContext = core.GetGLContext();
	
	for (auto& curr : pMesh->GetSubmeshes()) {
		pMaterial->Prepare(curr, modelTransform);
		curr.BindVAO();
		glContext.DrawElements(curr.GetPrimitiveType(), curr.GetNumOfIndices(), curr.indexTypeEnum);
	}
}

void GraphicalObject::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f GraphicalObject::GetTransform() const
{
	return modelTransform;
}
