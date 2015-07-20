#pragma once

#include "all_gl_headers.hpp"
#include "AttributeCategory.hpp"

class Mesh
{
public:
	using IndexType = GLuint;
	static const gl::enums::DataType indexTypeEnum = gl::enums::DataType::UnsignedInt;

public:
	Mesh();

	Mesh(Mesh&) = delete;
	Mesh& operator=(Mesh&) = delete;

	Mesh(Mesh&&);
	Mesh& operator=(Mesh&&);

	void LoadFromFile(const std::string& filename);

	void SetIndices(const std::vector<IndexType>& indexArray);
	void SetPrimitiveType(gl::enums::PrimitiveType type);
	void SetVertexAttributeElementDimensions(AttributeCategory targetAttribute, const int dimensionCount);
	void SetVertexAttributeBuffer(AttributeCategory targetAttribute, const gl::BufferData& dataArray);

	gl::enums::PrimitiveType GetPrimitiveType() const;
	GLsizei GetNumOfIndices() const;

	void AttachVertexAttribute(const AttributeCategory targetAttribute, const gl::Program& shaderProgram, const std::string& nameInShader) const;
	//void AttachVertexAttribute(const AttributeCategory targetAttribute, const int elementDimensionCount, const gl::Program& shaderProgram, const std::string& nameInShader) const;

	void BindVAO() const;

	static Mesh GenerateTriangle(float size);
	static Mesh GenerateCircle(float radius, int resolution);

private:
	struct VertexAttributeContainer
	{
		gl::Buffer buffer;
		int elementDimension;

		VertexAttributeContainer(gl::Buffer&& buffer, const int elementDimension) : buffer(std::move(buffer)), elementDimension(elementDimension) {}
		VertexAttributeContainer(VertexAttributeContainer&) = delete;
		void operator=(VertexAttributeContainer&) = delete;
		VertexAttributeContainer(VertexAttributeContainer&& r) : buffer(std::move(r.buffer)), elementDimension(r.elementDimension){}
	};

private:
	gl::VertexArray VAO;
	gl::Buffer indices;

	std::map<AttributeCategory,  VertexAttributeContainer> vertexAttributes;

	gl::enums::PrimitiveType primitiveType;
};

class OBJMesh
{
private:
	//int posDimensionCount;
	//int texCoordDimensionCount;

	std::vector<Mesh::IndexType> indices;

	std::map<AttributeCategory,  std::vector<gl::Vec3f> > vertexAttributes;

public:
	OBJMesh(std::istream& objContent);

	std::vector<Mesh::IndexType>& GetIndices();

	std::vector<GLfloat> GetVertexAttribute(AttributeCategory target);

private:
	
	template<typename elementType>
	std::vector<GLfloat> GetFloatVector(const std::vector<elementType>& input, int inputDimensions)
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

