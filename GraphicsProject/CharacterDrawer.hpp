#pragma once

/*
#include "all_gl_headers.hpp"

#include <SFML/Graphics.hpp>

#include "Mesh.hpp"

class CharacterDrawer
{
public:
	CharacterDrawer();

	void SetScreenResolution(gl::Vec2i res);

	//void DrawCharacter(gl::Context& glContext, const sf::Texture& fontTexture, const sf::Glyph& glyph, const sf::Vector2f pos);
	void DrawCharacter(gl::Context& glContext, const sf::Font& font, unsigned int character, unsigned int characterSize, const sf::Vector2f pos);
	
private:

	gl::Mat4f projectionMatrix;
	gl::Uniform<gl::Mat4f> sh_MVP;
	gl::Uniform<gl::Vec2f> sh_texCoordMin;
	gl::Uniform<gl::Vec2f> sh_texCoordMax;

	gl::Program shaderProgram;
	Mesh mesh;
};

*/