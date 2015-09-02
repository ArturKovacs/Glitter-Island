#pragma once

#include "all_gl_headers.hpp"
#include <list>

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "SimpleColoredDrawer.hpp"
#include "TextDrawer.hpp"
#include "BaseDemoContext.hpp"
#include "EditorContext.hpp"
#include "ContextManager.hpp"
#include "GraphicsEngine.hpp"

class DemoCore
{
public:
	static const std::string shadersFolderPath;
	static const std::string imgFolderPath;
	static const std::string modelsFolderPath;

public: //TODO These members are only made public temporarily! Make them private asap
	sf::Font overlayFont;
	TextDrawer textDrawer;
	Mesh circle;

public:
	DemoCore(sf::Window* pWindow);

	int Start();
	void Stop();
	
	sf::Time GetElapsedTime();

	int GetScreenWidth() const;
	int GetScreenHeight() const;

	float GetMouseSensitivity();
	
	GraphicsEngine& GetGraphicsEngine();

	sf::Window* GetWindow();

	void SaveAll();

private: // graphical state
	//int screenWidth;
	//int screenHeight;

	sf::Window* pWindow;
	
	GraphicsEngine graphicsEngine;

private: // misc
	bool running;
	sf::Clock clock;
	double elapsedSec;
	float currFPS;
	ContextManager contextManager;
	BaseDemoContext baseDemoContext;

	const float mouseSensitivity;

private:
	void CoreDraw();
};
