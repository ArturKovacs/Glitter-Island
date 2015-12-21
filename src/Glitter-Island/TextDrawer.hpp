#pragma once

#include "all_gl_headers.hpp"

#include <SFML/Graphics.hpp>

#include <GE/Mesh.hpp>

class TextDrawer
{
public:
	static float GetTextWidth(const sf::Text& text);
	static float GetTextHeight(const sf::Text& text);

public:
	TextDrawer();

	void SetScreenResolution(glm::ivec2 res);
	void Draw(gl::Context& glContext, const sf::Text& text);
	void DrawBackground(gl::Context& glContext, const sf::Text& text, const sf::Color& color, const float border = 0);
	void DrawAsList(gl::Context& glContext, const sf::Text& text, const int highlightedRowID, const sf::Color& highlightedColor);

private:
	glm::mat4 projectionMatrix;

	gl::Program characterShader;
	gl::Uniform<glm::mat4> sh_char_MVP;
	gl::Uniform<glm::vec2> sh_char_texCoordMin;
	gl::Uniform<glm::vec2> sh_char_texCoordMax;
	gl::Uniform<glm::vec4> sh_char_characterColor;

	gl::Program backgroundShader;
	gl::Uniform<glm::mat4> sh_bg_MVP;
	gl::Uniform<glm::vec4> sh_bg_color;

	Mesh quadMesh;

private:
	static float GetLineSpacing(const sf::Font& font, unsigned int characterSize);
};
