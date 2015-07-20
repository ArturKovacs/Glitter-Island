#pragma once

#include "all_gl_headers.hpp"
#include <list>

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "TextDrawer.hpp"
//#include "CharacterDrawer.hpp"
#include "Framebuffer.hpp"
#include "DefaultFramebuffer.hpp"
#include "GraphicalObject.hpp"
#include "DirectionalLight.hpp"
#include "Terrain.hpp"
#include "Skybox.hpp"
#include "Water.hpp"
#include "Camera.hpp"
#include "Fog.hpp"

enum class SelectedTool { NO_TOOL, PAINT_FLAT_SAND, PAINT_SAND_TEXTURE, PAINT_GRASS_TEXTURE, SPAWN_GRASS_BUNCH, SPAWN_ROCK_BUNCH, PLACE_MODEL };

class DemoCore
{
public:
	static const std::string shadersFolderPath;
	static const std::string imgFolderPath;
	static const std::string modelsFolderPath;

public:
	DemoCore(sf::Window* pWindow);

	int Start();

	void PushFramebuffer();
	void PopFramebuffer();

	void CopyFramebufferContents(const Framebuffer& source);

	Framebuffer& GetCurrentFramebuffer();
	gl::Context& GetGLContext();
	sf::Time GetElapsedTime();
	bool GetWireframeModeEnabled() const;

	DirectionalLight& GetSun();
	Camera& GetCamera();

	const DirectionalLight& GetSun() const;
	const Camera& GetCamera() const;

	static gl::Program LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name);

private: // graphical state
	int screenWidth;
	int screenHeight;

	sf::Window* pWindow;

	gl::Context glContext;

	std::list<Framebuffer> framebuffers;
	gl::DefaultFramebuffer defaultFBO;

	gl::Program finalFramebufferCopy;
	gl::Uniform<GLint> framebufferCopy_ScreenWidth;
	gl::Uniform<GLint> framebufferCopy_ScreenHeight;

private: // misc
	bool running;
	sf::Clock clock;
	double elapsedSec;
	sf::Font overlayFont;
	TextDrawer textDrawer;

private: //edit mode
	bool isInEditMode;
	SelectedTool selectedTool;

private: // demo properties, user state
	const float mouseSensitivity;
	const float camSpeed;
	const float fastSpeedMultiplyer;
	bool isInFastMode;

	bool wireframeModeEnabled;

	bool isTrackingMouse;
	float forwardMovement;
	float rightMovement;

private: // scene, objects
	const float terrainSize;

	DirectionalLight sun;
	Camera cam;

	Terrain terrain;
	Skybox skybox;
	Water water;
	Fog fog;

	std::vector<GraphicalObject> graphicalObjects;

private:
	void ClearFramebufferStack();

	void Resize(const int width, const int height);
	void MouseMoved();
	void KeyPressed(sf::Event::KeyEvent key);
	void KeyReleased(sf::Event::KeyEvent key);

	void Draw();
	void DrawScene();
	void DrawObjects();
	void DrawEditMode();
	void DrawOverlay();
};