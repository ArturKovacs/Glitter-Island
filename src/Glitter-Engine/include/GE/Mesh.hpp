#pragma once

#include "all_gl_headers.hpp"
#include "AttributeCategory.hpp"
#include "Camera.hpp"

class Material;
class DemoCore;

class Mesh
{
	friend class MeshManager;

public:
	class Submesh;

public:
	static Mesh GenerateTriangle(float size);
	static Mesh GenerateCircle(float radius, int resolution);
	static Mesh GenerateRectangle(float sizeX, float sizeY);
	static Mesh GenerateFrustum(const Frustum& frustum);

public:
	Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&&) = default;
	Mesh& operator=(Mesh&&) = default;

	std::vector<Submesh>& GetSubmeshes();
	const std::vector<Submesh>& GetSubmeshes() const;

private:
	std::vector<Submesh> submeshes;
};

class Mesh::Submesh
{
public:
	using IndexType = GLuint;
	static const gl::enums::DataType indexTypeEnum = gl::enums::DataType::UnsignedInt;

public:
	Submesh();

	Submesh(const Submesh&) = delete;
	Submesh& operator=(const Submesh&) = delete;

	Submesh(Submesh&&) = default;
	Submesh& operator=(Submesh&&) = default;

	void SetIndices(const std::vector<IndexType>& indexArray);
	void SetPrimitiveType(gl::enums::PrimitiveType type);
	void SetVertexAttributeElementDimensions(AttributeCategory targetAttribute, const int dimensionCount);
	void SetVertexAttributeBuffer(AttributeCategory targetAttribute, const gl::BufferData& dataArray);
	void SetMaterial(Material* newMaterial);
	Material* GetMaterial();

	gl::enums::PrimitiveType GetPrimitiveType() const;
	GLsizei GetNumOfIndices() const;

	void AttachVertexAttribute(const AttributeCategory targetAttribute, const gl::Program& shaderProgram, const std::string& nameInShader) const;

	void BindVAO() const;

private:
	struct VertexAttributeContainer
	{
		gl::Buffer buffer;
		int elementDimension;

		VertexAttributeContainer(gl::Buffer&& buffer, const int elementDimension) : buffer(std::move(buffer)), elementDimension(elementDimension) {}
		VertexAttributeContainer(const VertexAttributeContainer&) = delete;
		void operator=(const VertexAttributeContainer&) = delete;
		VertexAttributeContainer(VertexAttributeContainer&& r) : buffer(std::move(r.buffer)), elementDimension(r.elementDimension){}
	};

private:
	Material* pMaterial;

	gl::VertexArray VAO;
	gl::Buffer indices;

	std::map<AttributeCategory, VertexAttributeContainer> vertexAttributes;

	gl::enums::PrimitiveType primitiveType;
};
