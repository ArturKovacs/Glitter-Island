#pragma once

#include "all_gl_headers.hpp"
#include <list>

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "SimpleColoredDrawer.hpp"
#include "TextDrawer.hpp"
#include "Framebuffer.hpp"
#include "DefaultFramebuffer.hpp"
#include "GraphicalObject.hpp"
#include "DirectionalLight.hpp"
#include "Terrain.hpp"
#include "Skybox.hpp"
#include "Water.hpp"
#include "Camera.hpp"
#include "Fog.hpp"

class DemoCore
{
public:
	enum class EditorTool { NO_TOOL, PAINT_FLAT_SAND, PAINT_SAND_TEXTURE, PAINT_GRASS_TEXTURE, SPAWN_GRASS_BUNCH, SPAWN_ROCK_BUNCH, PLACE_MODEL };
	enum class EditorToolType { NO_TOOL, PAINT, SPAWN, PLACE };

	static const std::string shadersFolderPath;
	static const std::string imgFolderPath;
	static const std::string modelsFolderPath;

public:
	static EditorToolType GetToolType(EditorTool tool);
	static gl::Program LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name);

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
	SimpleColoredDrawer simpleColoredDrawer;
	Mesh circle;

private: //edit mode
	bool isInEditorMode;
	EditorTool selectedTool;
	gl::Vec4f pointPosAtCursor;

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
	const float waterLevel;
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

	void UpdatePointPosAtCursor();
	//gl::Vec4f GetPointPosAtPixel(sf::Vector2i pixelPos) const;

	void Update();

	void Draw();
	void DrawScene();
	void DrawObjects();
	void DrawEditorMode();
	void DrawOverlay();
};
