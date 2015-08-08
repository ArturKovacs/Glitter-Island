#include "Mesh.hpp"

#include <limits>
#include <string>
#include <sstream>
#include <functional>
#include <cctype>

#include "DemoCore.hpp"

Mesh::Mesh(){}

Mesh Mesh::GenerateTriangle(float size)
{
	Mesh result;
	Submesh submsh;

	std::vector<GLfloat> vertexPos = {
		0.0f, 0.0f, 0.0f,
		size, 0.0f, 0.0f,
		0.0f, size, 0.0f
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPos);

	std::vector<GLfloat> vertexNormal = {
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, vertexNormal);

	std::vector<Submesh::IndexType> indexData = {0, 1, 2};
	submsh.SetIndices(indexData);

	submsh.SetPrimitiveType(gl::enums::PrimitiveType::Triangles);

	result.submeshes.push_back(std::move(submsh));
	return std::move(result);
}

Mesh Mesh::GenerateCircle(float radius, int resolution)
{
	Mesh result;
	Submesh submsh;

	std::vector<gl::Vec2f> vertexPosData(resolution);
	for (int i = 0; i < resolution; i++) {
		float currAngle = (float(i)/resolution)*2*gl::math::Pi();
		vertexPosData.at(i) = gl::Vec2f(std::cos(currAngle)*radius, std::sin(currAngle)*radius);
	}
	submsh.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);
	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPosData);

	std::vector<Submesh::IndexType> indexData(resolution);
	for (int i = 0; i < resolution; i++) {
		indexData.at(i) = i;
	}
	submsh.SetIndices(indexData);

	submsh.SetPrimitiveType(gl::enums::PrimitiveType::LineLoop);

	result.submeshes.push_back(std::move(submsh));
	return std::move(result);
}

Mesh Mesh::GenerateRectangle(float sizeX, float sizeY)
{
	Mesh result;
	Submesh submsh;

	std::vector<gl::Vec2f> vertexPos = {
		gl::Vec2f(-sizeX*0.5f, -sizeY*0.5f),
		gl::Vec2f(+sizeX*0.5f, -sizeY*0.5f),
		gl::Vec2f(+sizeX*0.5f, +sizeY*0.5f),
		gl::Vec2f(-sizeX*0.5f, +sizeY*0.5f),
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPos);
	submsh.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);

	std::vector<gl::Vec3f> vertexNormal = {
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, vertexNormal);
	submsh.SetVertexAttributeElementDimensions(AttributeCategory::NORMAL, 3);

	std::vector<Submesh::IndexType> indexData = {0, 1, 2, 3};
	submsh.SetIndices(indexData);

	submsh.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);

	result.submeshes.push_back(std::move(submsh));
	return std::move(result);
}

Mesh::Mesh(Mesh&& r) :
submeshes(std::move(r.submeshes))
{}

Mesh& Mesh::operator=(Mesh&& r)
{
	submeshes = std::move(r.submeshes);

	return *this;
}

std::vector<Mesh::Submesh>& Mesh::GetSubmeshes()
{
	return submeshes;
}

const std::vector<Mesh::Submesh>& Mesh::GetSubmeshes() const
{
	return submeshes;
}

////////////////////////////////////////////////
//
// Mesh::Submesh
//
////////////////////////////////////////////////

Mesh::Submesh::Submesh() : pMaterial(nullptr), primitiveType(gl::enums::PrimitiveType::Points)
{
	ForEachAttribute([&](AttributeCategory current){
		vertexAttributes.insert(std::make_pair(current, VertexAttributeContainer(std::move(gl::Buffer()), GetAttributeCategoryInfo(current).defaultElementDimensions)));
	});
}

Mesh::Submesh::Submesh(Submesh&& r) :
pMaterial(r.pMaterial),
VAO(std::move(r.VAO)),
indices(std::move(r.indices)),
vertexAttributes(std::move(r.vertexAttributes)),
primitiveType(r.primitiveType)
{}

Mesh::Submesh& Mesh::Submesh::operator=(Submesh&& r)
{
	pMaterial = r.pMaterial;
	VAO = std::move(r.VAO);
	indices = std::move(r.indices);
	vertexAttributes = std::move(r.vertexAttributes);
	primitiveType = r.primitiveType;

	return *this;
}

void Mesh::Submesh::SetIndices(const std::vector<IndexType>& indexArray)
{
	VAO.Bind();

	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexArray);
}

void Mesh::Submesh::SetPrimitiveType(gl::enums::PrimitiveType type)
{
	primitiveType = type;
}

void Mesh::Submesh::SetVertexAttributeElementDimensions(AttributeCategory targetAttribute, const int dimensionCount)
{
	vertexAttributes.at(targetAttribute).elementDimension = dimensionCount;
}

void Mesh::Submesh::SetVertexAttributeBuffer(AttributeCategory targetAttribute, const gl::BufferData& dataArray)
{
	VAO.Bind();

	vertexAttributes.at(targetAttribute).buffer.Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, dataArray);
}

void Mesh::Submesh::SetMaterial(Material* newMaterial)
{
	pMaterial = newMaterial;
}

Material* Mesh::Submesh::GetMaterial()
{
	return pMaterial;
}

gl::enums::PrimitiveType Mesh::Submesh::GetPrimitiveType() const
{
	return primitiveType;
}

GLsizei Mesh::Submesh::GetNumOfIndices() const
{
	return indices.Size(gl::Buffer::Target::ElementArray).get() / sizeof(IndexType);
}

void Mesh::Submesh::AttachVertexAttribute(const AttributeCategory targetAttribute, const gl::Program& shaderProgram, const std::string& nameInShader) const
{
	const auto& attributeContainer = vertexAttributes.at(targetAttribute);

	VAO.Bind();
	attributeContainer.buffer.Bind(gl::Buffer::Target::Array);
	gl::VertexArrayAttrib attribute(shaderProgram, nameInShader);
	attribute.Setup<GLfloat>(attributeContainer.elementDimension);
	attribute.Enable();
}

void Mesh::Submesh::BindVAO() const
{
	VAO.Bind();
}
