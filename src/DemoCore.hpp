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
#include "GUIContext.hpp"
#include "EditorContext.hpp"

class DemoCore : GUIContext
{
public:
	static const std::string shadersFolderPath;
	static const std::string imgFolderPath;
	static const std::string modelsFolderPath;

public:
	static gl::Program LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name);

public: //TODO These members are only made public temporarily! Make them private asap
	sf::Font overlayFont;
	TextDrawer textDrawer;
	SimpleColoredDrawer simpleColoredDrawer;
	Mesh circle;

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
	Terrain& GetTerrain();

	const DirectionalLight& GetSun() const;
	const Camera& GetCamera() const;

	sf::Window* GetWindow();

	void SaveAll();

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
	float currFPS;

	EditorContext editorContext;
	std::vector<GUIContext*> activeGUIStack;

private: //edit mode
	//Moved to context

private: // demo properties, user state
	enum class SpeedMode { NORMAL, FAST, ULTRA };

	const float mouseSensitivity;
	const float camSpeed;
	const float fastSpeedMultiplyer;
	const float ultraSpeedMultiplyer;
	SpeedMode currentSpeedMode;

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
	//Fog fog;

	std::vector<GraphicalObject> graphicalObjects;

private:
	float GetCurrentSpeed() const;

	void ClearFramebufferStack();

	void ContextManagerUpdate(float deltaSec);
	void ContextManagerDraw();
	void ContextManagerMouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent);
	void ContextManagerKeyPressed(sf::Event::KeyEvent key);
	void ContextManagerKeyReleased(sf::Event::KeyEvent key);
        
        void PassKeyPressed(GUIContext* sourceContext, sf::Event::KeyEvent key);
        void PassKeyReleased(GUIContext* sourceContext, sf::Event::KeyEvent key);

	void Resize(const int width, const int height);

	void EnteringContext() override;
	void LeavingContext() override;

	void MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent) override;
	void KeyPressed(sf::Event::KeyEvent key) override;
	void KeyReleased(sf::Event::KeyEvent key) override;

	void Update(float deltaSec) override;

	void Draw() override;
	void DrawOverlayElements() override;
	void DrawScene();
	void DrawObjects();
	void DrawEditorMode();
};
