#pragma once

#include "all_gl_headers.hpp"

#include <SFML/Graphics.hpp>

#include "Mesh.hpp"

class TextDrawer
{
public:
	TextDrawer();

	void SetScreenResolution(gl::Vec2i res);
	void Draw(gl::Context& glContext, const sf::Text& text);
	void DrawBackground(gl::Context& glContext, const sf::Text& text, const sf::Color& color, const float border = 0);
	void DrawAsList(gl::Context& glContext, const sf::Text& text, const int highlightedRowID, const sf::Color& highlightedColor);

private:
	gl::Mat4f projectionMatrix;

	gl::Program characterShader;
	gl::Uniform<gl::Mat4f> sh_char_MVP;
	gl::Uniform<gl::Vec2f> sh_char_texCoordMin;
	gl::Uniform<gl::Vec2f> sh_char_texCoordMax;
	gl::Uniform<gl::Vec4f> sh_char_characterColor;

	gl::Program backgroundShader;
	gl::Uniform<gl::Mat4f> sh_bg_MVP;
	gl::Uniform<gl::Vec4f> sh_bg_color;

	Mesh quadMesh;

private:
	static float GetLineSpacing(const sf::Font& font, unsigned int characterSize);
};
