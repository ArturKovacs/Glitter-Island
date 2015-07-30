#include "TextDrawer.hpp"

#include "DemoCore.hpp"

TextDrawer::TextDrawer()
{
	characterShader = DemoCore::LoadShaderProgramFromFiles("characterDraw_v.glsl", "characterDraw_f.glsl");
	characterShader.Use();

	try {
		sh_char_MVP = gl::Uniform<gl::Mat4f>(characterShader, "MVP");
		sh_char_texCoordMin = gl::Uniform<gl::Vec2f>(characterShader, "texCoordMin");
		sh_char_texCoordMax = gl::Uniform<gl::Vec2f>(characterShader, "texCoordMax");
		sh_char_characterColor = gl::Uniform<gl::Vec4f>(characterShader, "characterColor");
		gl::UniformSampler(characterShader, "fontTexture").Set(0);
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}

	backgroundShader = DemoCore::LoadShaderProgramFromFiles("simpleColored_v.glsl", "simpleColored_f.glsl");
	backgroundShader.Use();

	try {
		sh_bg_MVP = gl::Uniform<gl::Mat4f>(backgroundShader, "MVP");
		sh_bg_color = gl::Uniform<gl::Vec4f>(backgroundShader, "color");
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
	quadMesh.SetVertexAttributeBuffer(AttributeCategory::POSITION, pos);
	quadMesh.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);
	quadMesh.AttachVertexAttribute(AttributeCategory::POSITION, characterShader, "vertexPos");
	quadMesh.AttachVertexAttribute(AttributeCategory::POSITION, backgroundShader, "vertexPos");

	const std::vector<gl::Vec2f> texCoord = {
		gl::Vec2f(0., 1.),
		gl::Vec2f(0., 0.),
		gl::Vec2f(1., 0.),
		gl::Vec2f(1., 1.)
	};
	quadMesh.SetVertexAttributeBuffer(AttributeCategory::TEX_COORD, texCoord);
	//quadMesh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");

	const std::vector<Mesh::IndexType> indices = {
		0, 1, 2, 3
	};

	quadMesh.SetIndices(indices);
	quadMesh.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);
}

void TextDrawer::SetScreenResolution(gl::Vec2i res)
{
	projectionMatrix = gl::CamMatrixf::Ortho(0, res.x(), res.y(), 0, -1, 1);
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

	quadMesh.BindVAO();
	characterShader.Use();

	gl::Texture::Active(0);
	sf::Texture::bind(&fontTexture);

	const sf::Color& color = text.getColor();
	sh_char_characterColor.Set(gl::Vec4f(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));

	const float* ptr = text.getTransform().getMatrix();
	gl::Mat4f baseTransform = gl::Mat4f(
		ptr[0], ptr[4], ptr[8], ptr[12],
		ptr[1], ptr[5], ptr[9], ptr[13],
		ptr[2], ptr[6], ptr[10], ptr[14],
		ptr[3], ptr[7], ptr[11], ptr[15]);

	sf::Vector2f charOffset = sf::Vector2f(0, characterSize);

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

			sf::Vector2f charPos = charOffset + sf::Vector2f(glyph.bounds.left, glyph.bounds.top);
			gl::Mat4f MVP = projectionMatrix * baseTransform * gl::ModelMatrixf::Translation(charPos.x, charPos.y, 0) * gl::ModelMatrixf::Scale(glyph.textureRect.width, glyph.textureRect.height, 0);
			sh_char_MVP.Set(MVP);

			gl::Vec2f texCoordsMin(float(glyph.textureRect.left) / fontTexture.getSize().x, float(glyph.textureRect.top) / fontTexture.getSize().y);
			gl::Vec2f texCoordsMax = texCoordsMin + gl::Vec2f(float(glyph.textureRect.width) / fontTexture.getSize().x, float(glyph.textureRect.height) / fontTexture.getSize().y);

			sh_char_texCoordMin.Set(texCoordsMin);
			sh_char_texCoordMax.Set(texCoordsMax);

			//glContext.Disable(gl::Capability::CullFace);
			glContext.DrawElements(quadMesh.GetPrimitiveType(), quadMesh.GetNumOfIndices(), quadMesh.indexTypeEnum);

			charOffset.x += glyph.advance;
		}
	}
}

void TextDrawer::DrawBackground(gl::Context& glContext, const sf::Text& text, const sf::Color& color, const float border)
{
	sf::IntRect bgRect;

	bgRect.top = -border;
	bgRect.left = -border;

	std::string string = text.getString().toAnsiString();
	std::vector<sf::String> lines;

	std::size_t prevLineEnd = 0;
	std::size_t currLineEnd;

	while ((currLineEnd = string.find('\n', prevLineEnd)) != std::string::npos ) {
		lines.push_back(string.substr(prevLineEnd, currLineEnd-prevLineEnd));
		prevLineEnd = currLineEnd+1;
	}

	const sf::Uint32 endPos = string.size();
	if (prevLineEnd != endPos) {
		lines.push_back(string.substr(prevLineEnd, endPos-prevLineEnd));
	}

	unsigned int charSize = text.getCharacterSize();
	bool bold = (text.getStyle() & sf::Text::Bold) != 0;

	float maxWidth = 0;
	for (const auto& currLine : lines) {
		float widthAccumulator = 0;
		sf::Uint32 prevChar = 0;
		for (const auto currChar : currLine) {
			const auto& glyph = text.getFont()->getGlyph(currChar, charSize, bold);
			widthAccumulator += text.getFont()->getKerning(prevChar, currChar, charSize);
			widthAccumulator += glyph.advance;
			prevChar = currChar;
		}

		maxWidth = std::max(maxWidth, widthAccumulator);
	}

	const float lineSpacing = GetLineSpacing(*text.getFont(), charSize);
	bgRect.height = lines.size() * (charSize + lineSpacing) - lineSpacing + 2*border;
	bgRect.width = maxWidth + 2*border;

	const float* ptr = text.getTransform().getMatrix();
	gl::Mat4f baseTransform = gl::Mat4f(
		ptr[0], ptr[4], ptr[8], ptr[12],
		ptr[1], ptr[5], ptr[9], ptr[13],
		ptr[2], ptr[6], ptr[10], ptr[14],
		ptr[3], ptr[7], ptr[11], ptr[15]);

	backgroundShader.Use();

	gl::Mat4f MVP = projectionMatrix * baseTransform * gl::ModelMatrixf::Translation(bgRect.left, bgRect.top, 0) * gl::ModelMatrixf::Scale(bgRect.width, bgRect.height, 0);
	sh_bg_MVP.Set(MVP);

	sh_bg_color.Set(gl::Vec4f(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));

	quadMesh.BindVAO();
	glContext.DrawElements(quadMesh.GetPrimitiveType(), quadMesh.GetNumOfIndices(), quadMesh.indexTypeEnum);
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

float TextDrawer::GetLineSpacing(const sf::Font& font, unsigned int characterSize)
{
	return font.getLineSpacing(characterSize)*0.5;
}
