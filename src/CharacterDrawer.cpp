#include "CharacterDrawer.hpp"
/*
#include "FileLoad.hpp"
#include "DemoCore.hpp"

CharacterDrawer::CharacterDrawer()
{
	gl::VertexShader vs;
	vs.Source(LoadFileAsString(DemoCore::shadersFolderPath + "characterDraw_v.glsl"));
	vs.Compile();

	gl::FragmentShader fs;
	fs.Source(LoadFileAsString(DemoCore::shadersFolderPath + "characterDraw_f.glsl"));
	fs.Compile();

	shaderProgram.AttachShader(vs);
	shaderProgram.AttachShader(fs);
	shaderProgram.Link();
	shaderProgram.Use();

	try {
		sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
		sh_texCoordMin = gl::Uniform<gl::Vec2f>(shaderProgram, "texCoordMin");
		sh_texCoordMax = gl::Uniform<gl::Vec2f>(shaderProgram, "texCoordMax");
		gl::UniformSampler(shaderProgram, "fontTexture").Set(0);
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	const std::vector<gl::Vec2f> pos = {
		gl::Vec2f(0., 0.),
		gl::Vec2f(0., 1.),
		gl::Vec2f(1., 1.),
		gl::Vec2f(1., 0.)
	};
	mesh.SetVertexAttributeBuffer(AttributeCategory::POSITION, pos);
	mesh.AttachVertexAttribute(AttributeCategory::POSITION, 2, shaderProgram, "vertexPos");

	const std::vector<gl::Vec2f> texCoord = {
		gl::Vec2f(0., 1.),
		gl::Vec2f(0., 0.),
		gl::Vec2f(1., 0.),
		gl::Vec2f(1., 1.)
	};
	mesh.SetVertexAttributeBuffer(AttributeCategory::TEX_COORD, texCoord);
	//mesh.AttachVertexAttribute(AttributeCategory::TEX_COORD, 2, shaderProgram, "vertexTexCoord");

	const std::vector<Mesh::IndexType> indices = {
		0, 1, 2, 3
	};

	mesh.SetIndices(indices);
	mesh.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);
}

void CharacterDrawer::SetScreenResolution(gl::Vec2i res)
{
	projectionMatrix = gl::CamMatrixf::Ortho(0, res.x(), res.y(), 0, -1, 1);
	//projectionMatrix = gl::CamMatrixf::Ortho(0, res.x(), 0, res.y(), -1, 1);
}

void CharacterDrawer::DrawCharacter(gl::Context& glContext, const sf::Font& font, unsigned int character, unsigned int characterSize, const sf::Vector2f pos)
{	
	const sf::Texture& fontTexture = font.getTexture(characterSize);
	sf::Glyph glyph = font.getGlyph(character, characterSize, false);

	shaderProgram.Use();

	gl::Texture::Active(0);
	sf::Texture::bind(&fontTexture);

	gl::Mat4f MVP = projectionMatrix * gl::ModelMatrixf::Translation(pos.x, pos.y, 0) * gl::ModelMatrixf::Scale(glyph.textureRect.width, glyph.textureRect.height, 0);
	sh_MVP.Set(MVP);

	//std::cout << "l: " << glyph.textureRect.left << ", t: " << glyph.textureRect.top << std::endl;
	//std::cout << "w: " << glyph.textureRect.width << ", h: " << glyph.textureRect.height << std::endl;

	gl::Vec2f texCoordsMin(float(glyph.textureRect.left) / fontTexture.getSize().x, float(glyph.textureRect.top) / fontTexture.getSize().y);
	gl::Vec2f texCoordsMax = texCoordsMin + gl::Vec2f(float(glyph.textureRect.width) / fontTexture.getSize().x, float(glyph.textureRect.height) / fontTexture.getSize().y);

	sh_texCoordMin.Set(texCoordsMin);
	sh_texCoordMax.Set(texCoordsMax);

	mesh.BindVAO();
	glContext.Disable(gl::Capability::CullFace);
	glContext.DrawElements(mesh.GetPrimitiveType(), mesh.GetNumOfIndices(), mesh.indexTypeEnum);
}
*/
