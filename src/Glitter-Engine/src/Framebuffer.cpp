#include <GE/Framebuffer.hpp>

#include <GE/GraphicsEngine.hpp>

Framebuffer::Framebuffer() : Framebuffer(1, 1)
{}

Framebuffer::Framebuffer(const int width, const int height) : 
colorTexName("colorTex"), depthTexName("depthTex"), vertexPosName("vertexPos"), pShaderProgram(nullptr)
{
	colorTex.Bind(gl::Texture::Target::_2D);
	gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::Linear);
	gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);
	//gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::ClampToEdge);
	//gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::ClampToEdge);
	gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::MirroredRepeat);
	gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::MirroredRepeat);

	depthTex.Bind(gl::Texture::Target::_2D);
	gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::Linear);
	gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);
	gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::MirroredRepeat);
	gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::MirroredRepeat);
	gl::Texture::CompareMode(gl::Texture::Target::_2D, gl::enums::TextureCompareMode::CompareRefToTexture);

	SetResolution(width, height);

	fbo.Bind(gl::Framebuffer::Target::Draw);
	gl::Framebuffer::AttachTexture(gl::Framebuffer::Target::Draw, gl::FramebufferAttachment::Color, colorTex, 0);
	gl::Framebuffer::AttachTexture(gl::Framebuffer::Target::Draw, gl::FramebufferAttachment::Depth, depthTex, 0);

	VAO.Bind();

	std::vector<GLfloat> posData = {
		-1, -1,
		+1, -1,
		+1, +1,
		-1, +1
	};

	vertexPos.Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, posData);

	std::vector<GLushort> indexData = {
		0, 1, 2, 3
	};

	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexData);
}

Framebuffer::Framebuffer(Framebuffer&& other) :
fbo(std::move(other.fbo)),
colorTex(std::move(other.colorTex)),
depthTex(std::move(other.depthTex)),
VAO(std::move(other.VAO)),
vertexPos(std::move(other.vertexPos)),
indices(std::move(other.indices)),
vertexPosName(std::move(other.vertexPosName)),
pShaderProgram(other.pShaderProgram)
{}

const gl::Texture& Framebuffer::GetColorTexture() const
{
	return colorTex;
}

const gl::Texture& Framebuffer::GetDepthTexture() const
{
	return depthTex;
}

void Framebuffer::SetShaderProgram(const gl::Program* program)
{
	VAO.Bind();

	pShaderProgram = program;

	pShaderProgram->Use();

	gl::UniformSampler(*pShaderProgram, colorTexName).Set(0);
	gl::UniformSampler(*pShaderProgram, depthTexName).Set(1);

	vertexPos.Bind(gl::Buffer::Target::Array);
	gl::VertexArrayAttrib attrib(*pShaderProgram, vertexPosName);
	attrib.Setup<GLfloat>(2);
	attrib.Enable();
}

void Framebuffer::SetResolution(const int width, const int height)
{
	//frameWidth = width;
	//frameHeight = height;

	gl::Texture::Bind(gl::Texture::Target::_2D, colorTex);
	gl::Texture::Image2D(gl::Texture::Target::_2D,
		0,
		gl::PixelDataInternalFormat::RGBA16F,
		width, height,
		0,
		gl::PixelDataFormat::RGBA,
		gl::PixelDataType::Float,
		nullptr);

	gl::Texture::Bind(gl::Texture::Target::_2D, depthTex);
	gl::Texture::Image2D(gl::Texture::Target::_2D,
		0,
		gl::PixelDataInternalFormat::DepthComponent32,
		width, height,
		0,
		gl::PixelDataFormat::DepthComponent,
		gl::PixelDataType::Float,
		nullptr);
}

void Framebuffer::Bind(gl::Framebuffer::Target target) const
{
	fbo.Bind(target);
}

void Framebuffer::Draw(GraphicsEngine& graphicsEngine)
{
	gl::Context& glContext = graphicsEngine.GetGLContext();
	if (pShaderProgram != nullptr) {
		pShaderProgram->Use();

		gl::Texture::Active(0);
		gl::Texture::Bind(gl::Texture::Target::_2D, colorTex);

		gl::Texture::Active(1);
		gl::Texture::Bind(gl::Texture::Target::_2D, depthTex);

		VAO.Bind();

		//Force frame to be drawn on the screen no matter what was there before.
		//Do not disable depth testing! Disabling depth testing will prevent shader from writing to the depth buffer.
		glContext.DepthFunc(gl::enums::CompareFunction::Always);

		glContext.PolygonMode(gl::enums::PolygonMode::Fill);
		glContext.DrawElements(gl::enums::PrimitiveType::TriangleFan, 4, gl::enums::DataType::UnsignedShort);
		if (graphicsEngine.GetWireframeModeEnabled()) {
			glContext.PolygonMode(gl::enums::PolygonMode::Line);
		}
		glContext.DepthFunc(gl::enums::CompareFunction::LEqual);
	}
}

void Framebuffer::SetVertexPosName(const std::string& name)
{
	vertexPosName = name;
}

void Framebuffer::SetColorTexName(const std::string& name)
{
	colorTexName = name;
}

void Framebuffer::SetDepthTexName(const std::string& name)
{
	depthTexName = name;
}
