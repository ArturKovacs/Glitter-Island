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

	/*
	template<typename elementType>
	void SetVertexAttributeBuffer(AttributeCategory targetAttribute, const std::vector<elementType>& dataArray)
	{
		VAO.Bind();

		vertexAttributes.at(targetAttribute).buffer.Bind(gl::Buffer::Target::Array);
		gl::Buffer::Data(gl::Buffer::Target::Array, dataArray);
	}*/
	
	void SetVertexAttributeBuffer(AttributeCategory targetAttribute, const gl::BufferData& dataArray);

	gl::enums::PrimitiveType GetPrimitiveType() const;
	GLsizei GetNumOfIndices() const;

	void AttachVertexAttribute(const AttributeCategory target, const gl::Program& shaderProgram, const std::string& nameInShader) const;

	void BindVAO() const;

	static Mesh GenerateTriangle(float size);

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

//private:
	//GLfloat* GetPosition();
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
	//int GetPosIndexForList(int objPosIndex) { return (objPosIndex - 1)*posDimensionCount; }
	//int GetTexCoordIndexForList(int objTexCoordIndex) { return (objTexCoordIndex - 1)*texCoordDimensionCount; }
	//int GetNormalIndexForList(int objNormalIndex) { return (objNormalIndex - 1)*3; }

	
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

