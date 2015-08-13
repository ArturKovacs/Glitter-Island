#include "DepthOnlyMaterial.hpp"

#include "DemoCore.hpp"

DepthOnlyMaterial::DepthOnlyMaterial(DemoCore* pCore) : pCore(pCore)
{
	shaderProgram = DemoCore::LoadShaderProgramFromFiles("DepthOnly_v.glsl", "DepthOnly_f.glsl");
	shaderProgram.Use();

	sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
	gl::UniformSampler(shaderProgram, "texContainingAlpha").Set(0);

	std::array<GLfloat, 4> pixelData = { 1, 1, 1, 1 };

	defaultTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::MinFilter(gl::Texture::Target::_2D, gl::TextureMinFilter::Linear);
	gl::Texture::MagFilter(gl::Texture::Target::_2D, gl::TextureMagFilter::Linear);
	gl::Texture::WrapS(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);
	gl::Texture::WrapT(gl::Texture::Target::_2D, gl::TextureWrap::Repeat);

	gl::Texture::Image2D(gl::Texture::Target::_2D,
		0,
		gl::enums::PixelDataInternalFormat::SRGB8Alpha8,
		1,
		1,
		0,
		gl::enums::PixelDataFormat::RGBA,
		gl::DataType::Float,
		pixelData.data());

	pTextureContainingAlpha = &defaultTexture;
}

DepthOnlyMaterial::~DepthOnlyMaterial()
{}

const gl::Texture* DepthOnlyMaterial::GetTextureContainigAlpha() const
{
	return pTextureContainingAlpha;
}

void DepthOnlyMaterial::Prepare(Mesh::Submesh& submsh, gl::Mat4f& modelTransform)
{
	shaderProgram.Use();

	submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
	submsh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "texCoord");

	gl::Texture::Active(0);
	pTextureContainingAlpha->Bind(gl::Texture::Target::_2D);

	sh_MVP.Set(pCore->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
}

void DepthOnlyMaterial::SetTextureContainingAlpha(const gl::Texture* pTexture)
{
	if (pTexture != nullptr) {
		pTextureContainingAlpha = pTexture;
	}
}

void DepthOnlyMaterial::ResetTextureContainingAlpha()
{
	pTextureContainingAlpha = &defaultTexture;
}
