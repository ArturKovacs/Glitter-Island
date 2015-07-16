#pragma once

#include "all_gl_headers.h"

#if (0)

class GraphicalObject
{
public:
	struct ShaderVarId
	{
		std::string name;
		ShaderVarId(const std::string& name) : name(name){}

		bool operator<(const ShaderVarId& other) const { return name < other.name; }
		bool operator==(const ShaderVarId& other) const { return name == other.name; }
	};

	struct VertexAttribId : public ShaderVarId
	{
		int elementDimension;
		VertexAttribId(const std::string& name, const int elementDimension) : ShaderVarId(name), elementDimension(elementDimension) {}
	};

	typedef ShaderVarId ShaderUniformId;

	static const ShaderUniformId UNIFORM_MVP_ID;
	static const ShaderUniformId UNIFORM_MODEL_TRANSPOSED_INVERSE_ID;

	static std::string GetVertexShaderSourceInputAttributeString();

protected:
	struct VertexAttributeBufferContainer
	{
		//Buffer is non-copyable, so this is my solution... (have a better idea?)
		std::shared_ptr<gl::Buffer> buffer;
		bool enabled;

		VertexAttributeBufferContainer(gl::Buffer* buffer) : buffer(buffer), enabled(false){}
	};

	template <typename UniformType>
	struct UniformContainer
	{
		gl::Uniform<UniformType> shaderUniform;
		bool enabled;

		UniformContainer() : enabled(false) {}
	};

private:
	gl::VertexArray VAO;
	gl::Buffer indices;

	std::map<VertexAttribId, VertexAttributeBufferContainer> vertexAttributes;
	std::map<ShaderUniformId, UniformContainer<gl::Mat4f> > matrixUniforms; 

	const gl::Program* pShaderProgram;

	gl::Mat4f modelTransform;

	gl::enums::PrimitiveType primitiveType;

public:
	typedef GLuint IndexType;
	static const gl::enums::DataType indexTypeEnum = gl::enums::DataType::UnsignedInt;

public:
	GraphicalObject();

	void Draw(const gl::Context& glContext, const gl::Mat4f& viewProjectionTransform);

	void SetIndices(const std::vector<IndexType>& indexArray);
	void SetPrimitiveType(gl::enums::PrimitiveType type);

	void SetShaderProgram(const gl::Program* pProgram);
	const gl::Program* GetShaderProgram() const;

	void AddVertexAttribute(const VertexAttribId& newAttribute);
	void AddVertexAttribute(const VertexAttribId& newAttribute, const std::vector<GLfloat>& dataArray);
	void SetVertexAttributeBuffer(const VertexAttribId& targetAttribute, const std::vector<GLfloat>& dataArray);

	void SetTransform(const gl::Mat4f& transform);
	gl::Mat4f GetTransform() const;

	void GenerateTriangle(float size);
};

#endif
