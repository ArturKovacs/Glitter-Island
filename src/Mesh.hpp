#pragma once

#include "all_gl_headers.hpp"
#include "AttributeCategory.hpp"

class Material;
class DemoCore;

class Mesh
{
	// Can not make only the mesh loader function friend,
	// because DemoCore can not be included for this file. (would cause circular inclusion)
	// Using forward declaration instead.
	friend class DemoCore;

public:
	class Submesh;

public:
	static Mesh GenerateTriangle(float size);
	static Mesh GenerateCircle(float radius, int resolution);
	static Mesh GenerateRectangle(float sizeX, float sizeY);

public:
	Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&&);
	Mesh& operator=(Mesh&&);

	std::vector<Submesh>& GetSubmeshes();
	const std::vector<Submesh>& GetSubmeshes() const;

	//const std::vector<std::string>& GetMtlLibFilenames() const;

private:
	std::vector<Submesh> submeshes;
	//std::vector<std::string> mtllibFilenames;

private:
	void LoadFromOBJFile(DemoCore* pCore, const std::string& filename);

	template<typename elementType>
	static std::vector<GLfloat> GetFloatVector(const std::vector<elementType>& input, int inputDimensions)
	{
		static_assert(std::is_same<elementType, gl::Vec2f>::value || std::is_same<elementType, gl::Vec3f>::value, "Unsupported type!");

		if (input.size() > 0) {
			assert(inputDimensions <= input.at(0).Size());
		}

		std::vector<GLfloat> result(input.size()*inputDimensions);

		int resultIndex = 0;
		for (const auto& current : input) {
			for (int i = 0; i < inputDimensions; i++) {
				result.at(resultIndex++) = current[i];
			}
		}

		return result;
	}
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

	Submesh(Submesh&&);
	Submesh& operator=(Submesh&&);

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
	//std::string materialName;
	Material* pMaterial;

	gl::VertexArray VAO;
	gl::Buffer indices;

	std::map<AttributeCategory, VertexAttributeContainer> vertexAttributes;

	gl::enums::PrimitiveType primitiveType;
};
