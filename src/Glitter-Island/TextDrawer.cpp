#include "TextDrawer.hpp"

#include "DemoCore.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

class CustomMatrix
{
private:
	float data[4][4];

public:
};

TextDrawer::TextDrawer()
{
	characterShader = GraphicsEngine::LoadShaderProgramFromFiles("CharacterDraw_v.glsl", "CharacterDraw_f.glsl");
	characterShader.Use();

	try {
		sh_char_MVP = gl::Uniform<glm::mat4>(characterShader, "MVP");
		sh_char_texCoordMin = gl::Uniform<glm::vec2>(characterShader, "texCoordMin");
		sh_char_texCoordMax = gl::Uniform<glm::vec2>(characterShader, "texCoordMax");
		sh_char_characterColor = gl::Uniform<glm::vec4>(characterShader, "characterColor");
		gl::UniformSampler(characterShader, "fontTexture").Set(0);
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	backgroundShader = GraphicsEngine::LoadShaderProgramFromFiles("SimpleColored_v.glsl", "SimpleColored_f.glsl");
	backgroundShader.Use();

	try {
		gl::Uniform<CustomMatrix> a;
		sh_bg_MVP = gl::Uniform<glm::mat4>(backgroundShader, "MVP");
		sh_bg_color = gl::Uniform<glm::vec4>(backgroundShader, "color");
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	Mesh::Submesh submsh;
	const std::vector<glm::vec2> pos = {
		glm::vec2(0., 0.),
		glm::vec2(0., 1.),
		glm::vec2(1., 1.),
		glm::vec2(1., 0.)
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, pos);
	submsh.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, characterShader, "vertexPos");
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, backgroundShader, "vertexPos");

	const std::vector<glm::vec2> texCoord = {
		glm::vec2(0., 1.),
		glm::vec2(0., 0.),
		glm::vec2(1., 0.),
		glm::vec2(1., 1.)
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::TEX_COORD, texCoord);
	//quadMesh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");

	const std::vector<Mesh::Submesh::IndexType> indices = {
		0, 1, 2, 3
	};

	submsh.SetIndices(indices);
	submsh.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);

	quadMesh.GetSubmeshes().push_back(std::move(submsh));
}

void TextDrawer::SetScreenResolution(glm::ivec2 res)
{
	projectionMatrix = glm::ortho<float>(0.f, float(res.x), float(res.y), 0.f, -1.f, 1.f);
	//projectionMatrix = gl::CamMatrixf::Ortho(0, res.x(), 0, res.y(), -1, 1);
}

void TextDrawer::Draw(gl::Context& glContext, const sf::Text& text)
{
	if (!text.getFont()) {
		return;
	}

	const sf::Font& font = *text.getFont();
	unsigned int characterSize = text.getCharacterSize();
	const sf::Texture& fontTexture = font.getTexture(characterSize);

	Mesh::Submesh& quad_submsh = quadMesh.GetSubmeshes().at(0);
	quad_submsh.BindVAO();
	characterShader.Use();

	gl::Texture::Active(0);
	sf::Texture::bind(&fontTexture);

	const sf::Color& color = text.getColor();
	sh_char_characterColor.Set(glm::vec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));

	const float* ptr = text.getTransform().getMatrix();
	glm::mat4 baseTransform = glm::mat4(
		ptr[0], ptr[1], ptr[2], ptr[3],
		ptr[4], ptr[5], ptr[6], ptr[7],
		ptr[8], ptr[9], ptr[10], ptr[11],
		ptr[12], ptr[13], ptr[14], ptr[15]);

	sf::Vector2f charOffset = sf::Vector2f(0.f, float(characterSize));

	const sf::String& string = text.getString();

	sf::Uint32 prevChar = 0;
	for (auto currChar : string) {

		if (currChar == L'\n') {
			charOffset.y += characterSize + GetLineSpacing(font, characterSize);
			charOffset.x = 0;
			prevChar = 0;
		}
		else {
			charOffset.x += font.getKerning(prevChar, currChar, characterSize);
			prevChar = currChar;

			sf::Glyph glyph = font.getGlyph(currChar, characterSize, (text.getStyle() & sf::Text::Bold) != 0);

			sf::Vector2f charPos = charOffset + sf::Vector2f(float(glyph.bounds.left), float(glyph.bounds.top));
			glm::mat4 MVP = glm::translate(glm::mat4(1.f), glm::vec3(charPos.x, charPos.y, 0));
			MVP = glm::scale(MVP, glm::vec3(glyph.textureRect.width, glyph.textureRect.height, 0));
			MVP = projectionMatrix * baseTransform * MVP;
			//glm::mat4 MVP = projectionMatrix * baseTransform * gl::ModelMatrixf::Translation(charPos.x, charPos.y, 0) * gl::ModelMatrixf::Scale(glyph.textureRect.width, glyph.textureRect.height, 0);
			sh_char_MVP.Set(MVP);

			glm::vec2 texCoordsMin(float(glyph.textureRect.left) / fontTexture.getSize().x, float(glyph.textureRect.top) / fontTexture.getSize().y);
			glm::vec2 texCoordsMax = texCoordsMin + glm::vec2(float(glyph.textureRect.width) / fontTexture.getSize().x, float(glyph.textureRect.height) / fontTexture.getSize().y);

			sh_char_texCoordMin.Set(texCoordsMin);
			sh_char_texCoordMax.Set(texCoordsMax);

			//glContext.Disable(gl::Capability::CullFace);
			glContext.DrawElements(quad_submsh.GetPrimitiveType(), quad_submsh.GetNumOfIndices(), quad_submsh.indexTypeEnum);

			charOffset.x += glyph.advance;
		}
	}
}

void TextDrawer::DrawBackground(gl::Context& glContext, const sf::Text& text, const sf::Color& color, const float border)
{
	sf::IntRect bgRect;

	bgRect.top = static_cast<int>(-border);
	bgRect.left = static_cast<int>(-border);

	unsigned int charSize = text.getCharacterSize();

	float maxWidth = GetTextWidth(text);

	const float lineSpacing = GetLineSpacing(*text.getFont(), charSize);
	bgRect.height = static_cast<int>(GetTextHeight(text) + 2*border);
	bgRect.width = static_cast<int>(maxWidth + 2*border);

	const float* ptr = text.getTransform().getMatrix();
	glm::mat4 baseTransform = glm::mat4(
		ptr[0], ptr[1], ptr[2], ptr[3],
		ptr[4], ptr[5], ptr[6], ptr[7],
		ptr[8], ptr[9], ptr[10], ptr[11],
		ptr[12], ptr[13], ptr[14], ptr[15]);

	backgroundShader.Use();

	glm::mat4 MVP = glm::translate(glm::mat4(1.f), glm::vec3(bgRect.left, bgRect.top, 0));
	MVP = glm::scale(MVP, glm::vec3(bgRect.width, bgRect.height, 0));
	MVP = projectionMatrix * baseTransform * MVP;
	//glm::mat4 MVP = projectionMatrix * baseTransform * gl::ModelMatrixf::Translation(bgRect.left, bgRect.top, 0) * gl::ModelMatrixf::Scale(bgRect.width, bgRect.height, 0);
	sh_bg_MVP.Set(MVP);

	sh_bg_color.Set(glm::vec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));

	Mesh::Submesh& quad_submsh = quadMesh.GetSubmeshes().at(0);
	quad_submsh.BindVAO();
	glContext.DrawElements(quad_submsh.GetPrimitiveType(), quad_submsh.GetNumOfIndices(), quad_submsh.indexTypeEnum);
}

void TextDrawer::DrawAsList(gl::Context& glContext, const sf::Text& text, const int highlightedRowID, const sf::Color& highlightedColor)
{
	sf::Text textToDraw = text;

	sf::String highlightedOnly;
	sf::String othersOnly;

	int currRowID = 0;
	for (auto currChar : text.getString()) {
		const bool isLineFeed = currChar == (unsigned int)'\n';

		if (currRowID != highlightedRowID || isLineFeed) {
			othersOnly += currChar;
		}
		if (currRowID == highlightedRowID || isLineFeed) {
			highlightedOnly += currChar;
		}

		if (isLineFeed) {
			currRowID++;
		}
	}

	textToDraw.setString(othersOnly);
	Draw(glContext, textToDraw);

	textToDraw.setString(highlightedOnly);
	textToDraw.setColor(highlightedColor);
	Draw(glContext, textToDraw);
}

float TextDrawer::GetTextWidth(const sf::Text& text)
{
	const sf::String& str = text.getString();

	unsigned int charSize = text.getCharacterSize();
	bool bold = (text.getStyle() & sf::Text::Bold) != 0;

	sf::Uint32 prevChar = 0;
	float maxWidth = 0;
	float widthAccumulator = 0;
	for (auto currChar : str) {
		if (currChar == (unsigned int)'\n') {
			maxWidth = std::max(maxWidth, widthAccumulator);
			widthAccumulator = 0;
			prevChar = 0;
		}
		else {
			const auto& glyph = text.getFont()->getGlyph(currChar, charSize, bold);
			widthAccumulator += text.getFont()->getKerning(prevChar, currChar, charSize);
			widthAccumulator += glyph.advance;
			prevChar = currChar;
		}
	}

	maxWidth = std::max(maxWidth, widthAccumulator);

	return maxWidth;
}

float TextDrawer::GetTextHeight(const sf::Text& text)
{
	const sf::String& str = text.getString();

	if (str.getSize() == 0) {
		return 0;
	}

	int lineCount = 1;
	for (auto currChar : str) {
		if (currChar == (unsigned int)'\n') {
			lineCount++;
		}
	}

	int charSize = text.getCharacterSize();
	float lineSpacing = GetLineSpacing(*text.getFont(), charSize);
	return (lineCount * (charSize + lineSpacing)) - lineSpacing;
}

float TextDrawer::GetLineSpacing(const sf::Font& font, unsigned int characterSize)
{
	return font.getLineSpacing(characterSize)*0.5f;
}
