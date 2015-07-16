#include "GraphicalObject.h"

#include "all_gl_headers.h"

#if (0) //put this graphical object away for now

//const GraphicalObject::VertexAttribId GraphicalObject::ATTRIB_VERTEX_POS_ID(std::string("vertexPos"), 3);
//const GraphicalObject::VertexAttribId GraphicalObject::ATTRIB_VERTEX_NORMAL_ID(std::string("vertexNormal"), 3);
//const GraphicalObject::VertexAttribId GraphicalObject::ATTRIB_VERTEX_TEX_COORD_ID(std::string("vertexTexCoord"), 2);

const GraphicalObject::ShaderUniformId GraphicalObject::UNIFORM_MVP_ID(std::string("MVP"));
const GraphicalObject::ShaderUniformId GraphicalObject::UNIFORM_MODEL_TRANSPOSED_INVERSE_ID(std::string("model_transposed_inverse"));

GraphicalObject::GraphicalObject() : pShaderProgram(nullptr)
{
	//vertexAttributes.insert(std::make_pair(ATTRIB_VERTEX_POS_ID, VertexAttributeBufferContainer(new gl::Buffer)));
	//vertexAttributes.insert(std::make_pair(ATTRIB_VERTEX_NORMAL_ID, VertexAttributeBufferContainer(new gl::Buffer)));
	//vertexAttributes.insert(std::make_pair(ATTRIB_VERTEX_TEX_COORD_ID, VertexAttributeBufferContainer(new gl::Buffer)));

	matrixUniforms.insert(std::make_pair(UNIFORM_MVP_ID, UniformContainer<gl::Mat4f>()));
	matrixUniforms.insert(std::make_pair(UNIFORM_MODEL_TRANSPOSED_INVERSE_ID, UniformContainer<gl::Mat4f>()));
}

static gl::Mat4f SajatTransposeFaszomMertNemMukodikAzOglPlusOsTODO(const gl::Mat4f& input)
{
	gl::Mat4f result;

	for (std::size_t i = 0; i != 4; ++i) {
		for (std::size_t j = 0; j != 4; ++j) {
			//((GLfloat*)result.Data())[i*4+j] = input.Data()[j*4+i];
			result.Set(i, j, input.At(j, i));
		}
	}

	return result;
}

void GraphicalObject::Draw(const gl::Context& glContext, const gl::Mat4f& viewProjectionTransform)
{
	if (pShaderProgram != nullptr) {
		VAO.Bind();

		//vertexPositions.Bind();

		pShaderProgram->Use();

		const gl::Mat4f MVP = viewProjectionTransform * modelTransform;
		if (matrixUniforms.at(UNIFORM_MVP_ID).enabled) {
			matrixUniforms.at(UNIFORM_MVP_ID).shaderUniform.Set(MVP);
		}
		if (matrixUniforms.at(UNIFORM_MODEL_TRANSPOSED_INVERSE_ID).enabled) {
			matrixUniforms.at(UNIFORM_MODEL_TRANSPOSED_INVERSE_ID).shaderUniform.Set(SajatTransposeFaszomMertNemMukodikAzOglPlusOsTODO(gl::Inverse(modelTransform)));
		}

		glContext.DrawElements(primitiveType, indices.Size(gl::Buffer::Target::ElementArray).get() / sizeof(IndexType), indexTypeEnum);
	}
}

void GraphicalObject::SetIndices(const std::vector<IndexType>& indexArray)
{
	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexArray); 
}

void GraphicalObject::SetPrimitiveType(gl::enums::PrimitiveType type)
{
	primitiveType = type;
}

void GraphicalObject::SetShaderProgram(const gl::Program* pProgram)
{
	VAO.Bind();
	pShaderProgram = pProgram;

	int exceptionCount = 0;
	std::string exceptionMessage;

	for (auto& current : matrixUniforms) {
		auto& identifier = current.first;
		auto& uniformContainer = current.second;

		if (uniformContainer.enabled) {
			try {
				uniformContainer.shaderUniform = gl::Uniform<gl::Mat4f>(*pShaderProgram, identifier.name);
			}
			catch (gl::Error& err) {
				exceptionCount++;
				exceptionMessage.append(err.what()).append("\n");
			}
		}
	}

	for (auto& current : vertexAttributes) {
		auto& identifier = current.first;
		auto& attributeContainer = current.second;

		if (attributeContainer.enabled) {
			try{
				attributeContainer.buffer.get()->Bind(gl::Buffer::Target::Array);
				gl::VertexArrayAttrib attribute(*pShaderProgram, identifier.name);
				attribute.Setup<GLfloat>(identifier.elementDimension);
				attribute.Enable();
			}
			catch (gl::Error& err) {
				exceptionCount++;
				exceptionMessage.append(err.what()).append("\n");
			}
		}
	}

	if (exceptionCount > 0) {
		throw gl::Error(exceptionMessage.c_str());
	}
}

const gl::Program* GraphicalObject::GetShaderProgram() const
{
	return pShaderProgram;
}

void GraphicalObject::AddVertexAttribute(const VertexAttribId& newAttribute)
{
	vertexAttributes.insert(std::make_pair(newAttribute, VertexAttributeBufferContainer(new gl::Buffer)));
}

void GraphicalObject::AddVertexAttribute(const VertexAttribId& newAttribute, const std::vector<GLfloat>& dataArray)
{
	gl::Buffer* bufferPointer = new gl::Buffer;
	vertexAttributes.insert(std::make_pair(newAttribute, VertexAttributeBufferContainer(bufferPointer)));
	
	VAO.Bind();
	bufferPointer->Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, dataArray);
}

void GraphicalObject::SetVertexAttributeBuffer(const VertexAttribId& targetAttribute, const std::vector<GLfloat>& dataArray)
{
	VAO.Bind();
	vertexAttributes.at(targetAttribute).buffer.get()->Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, dataArray);
}

void GraphicalObject::SetTransform(const gl::Mat4f& transform)
{
	modelTransform = transform;
}

gl::Mat4f GraphicalObject::GetTransform() const
{
	return modelTransform;
}

void GraphicalObject::GenerateTriangle(float size)
{
	std::vector<GLfloat> vertexData = {
		0.0f, 0.0f, 0.0f,
		size, 0.0f, 0.0f,
		0.0f, size, 0.0f
	};

	///gl::Buffer::Data(gl::Buffer::Target::Array, dataArray); //??

	std::vector<IndexType> indexData = {0, 1, 2};

	SetIndices(indexData);

	SetPrimitiveType(gl::enums::PrimitiveType::Triangles);
}

#endif
