#include <GE/Framebuffer.hpp>

#include <GE/GraphicsEngine.hpp>

Framebuffer::Framebuffer() : pShaderProgram(nullptr)
{}

Framebuffer::Framebuffer(const int width, const int height, uint8_t attachmentFlags) : 
vertexPosName("vertexPos"), pShaderProgram(nullptr), w(width), h(height)
{
	fbo.Bind(gl::Framebuffer::Target::Draw);

	for (uint8_t currFlag = 1; currFlag < _ATTACHMENT_END_; currFlag <<= 1) {
		if ((attachmentFlags & currFlag) != 0) {
			gl::Texture texture;
			texture.Bind(gl::Texture::Target::_2D);
			gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::Linear);
			gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);
			gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::MirroredRepeat);
			gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::MirroredRepeat);

			if ((currFlag & ATTACHMENT_DEPTH) != 0) {
				gl::Texture::CompareMode(gl::Texture::Target::_2D, gl::enums::TextureCompareMode::CompareRefToTexture);
				gl::Texture::CompareFunc(gl::Texture::Target::_2D, gl::enums::CompareFunction::LEqual);
			}

			textures[currFlag] = std::move(texture);
			gl::Framebuffer::AttachTexture(gl::Framebuffer::Target::Draw, GetGLAttachmentFromFlag((AttachmentFlag)currFlag), textures[currFlag], 0);
		}
	}

	SetResolution(width, height);

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

void Framebuffer::CopyFramebufferContents(const Framebuffer& source)
{
	this->Bind(gl::Framebuffer::Target::Draw);
	source.Bind(gl::Framebuffer::Target::Read);
	
	gl::Context::BlitFramebuffer(0, 0, source.w, source.h, 0, 0, w, h, gl::Bitfield<gl::enums::BufferSelectBit>(gl::enums::BufferSelectBit::DepthBuffer) |= gl::enums::BufferSelectBit::ColorBuffer, gl::enums::BlitFilter::Nearest);
	//glContext.BlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, gl::Bitfield<gl::enums::BufferSelectBit>(gl::enums::BufferSelectBit::DepthBuffer) |= gl::enums::BufferSelectBit::ColorBuffer, gl::enums::BlitFilter::Nearest);
}

const gl::Texture& Framebuffer::GetTexture(AttachmentFlag attachmentID) const
{
	return textures.at((uint8_t)attachmentID);
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

		for(auto& current : textures) {
			if ((usedAttachments & current.first) != 0) {
				try {
					gl::Texture::Active(textureShaderIDs.at(current.first).index);
					gl::Texture::Bind(gl::Texture::Target::_2D, current.second);
				}
				catch (std::out_of_range&) {
					throw std::out_of_range("Framebuffer was tried to be drawn with unknown attachments. Call SetTextureShaderID for the desired attachemnt before setting the shader.");
				}
			}
		}

		VAO.Bind();

		//Force frame to be drawn on the screen no matter what was there before.
		//Do not disable depth testing! Disabling depth testing will prevent shader from writing to the depth buffer.
		glContext.Enable(gl::Capability::DepthTest);
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

void Framebuffer::SetTextureShaderID(AttachmentFlag attachmentID, std::string name, int index)
{
	textureShaderIDs[(uint8_t)attachmentID] = std::move(TextureShaderID(name, index));
}

void Framebuffer::SetShaderProgram(const gl::Program* program, uint8_t usedAttachmentFlags)
{
	VAO.Bind();

	pShaderProgram = program;

	pShaderProgram->Use();
	
	usedAttachments = usedAttachmentFlags;
	
	for (auto& current : textureShaderIDs) {
		if ((usedAttachments & current.first) != 0) {
			gl::UniformSampler(*pShaderProgram, current.second.name).Set(current.second.index);
		}
	}

	vertexPos.Bind(gl::Buffer::Target::Array);
	gl::VertexArrayAttrib attrib(*pShaderProgram, vertexPosName);
	attrib.Setup<GLfloat>(2);
	attrib.Enable();
}

void Framebuffer::SetResolution(const int width, const int height)
{
	w = width;
	h = height;

	for(auto& current : textures) {
		gl::PixelDataInternalFormat internalFormat;
		gl::PixelDataFormat format;

		switch (current.first) {
		case ATTACHMENT_COLOR:
			internalFormat = gl::PixelDataInternalFormat::RGBA16F;
			format = gl::PixelDataFormat::RGBA;
			break;
		case ATTACHMENT_NORMAL:
			internalFormat = gl::PixelDataInternalFormat::RGB16F;
			format = gl::PixelDataFormat::RGB;
			break;
		case ATTACHMENT_VIEW_DEPTH:
			internalFormat = gl::PixelDataInternalFormat::R32F;
			format = gl::PixelDataFormat::Red;
			break;
		case ATTACHMENT_DEPTH:
			internalFormat = gl::PixelDataInternalFormat::DepthComponent32;
			format = gl::PixelDataFormat::DepthComponent;
			break;
		default:
			assert(false);
			break;
		}

		gl::Texture::Bind(gl::Texture::Target::_2D, current.second);
		gl::Texture::Image2D(gl::Texture::Target::_2D,
			0,
			internalFormat,
			width, height,
			0,
			format,
			gl::PixelDataType::Float,
			nullptr);
	}
}

gl::FramebufferAttachment Framebuffer::GetGLAttachmentFromFlag(AttachmentFlag flag)
{
	gl::FramebufferAttachment result;
	switch (flag) {
	case ATTACHMENT_COLOR:
		result = gl::FramebufferAttachment::Color;
		break;
	case ATTACHMENT_NORMAL:
		result = gl::FramebufferAttachment::Color1;
		break;
	case ATTACHMENT_VIEW_DEPTH:
		result = gl::FramebufferAttachment::Color2;
		break;
	case ATTACHMENT_DEPTH:
		result = gl::FramebufferAttachment::Depth;
		break;
	default:
		assert(false);
		break;
	}

	return result;
}
