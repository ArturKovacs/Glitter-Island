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
#include "BaseDemoContext.hpp"
#include "EditorContext.hpp"
#include "ContextManager.hpp"

class DemoCore
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
	void Stop();

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
	float GetMouseSensitivity();

	sf::Window* GetWindow();

	void AddGraphicalObject(GraphicalObject&& newObject);

	Mesh* LoadMeshFromFile(const std::string& filename);
	GraphicalObject LoadGraphicalObjectFromFile(const std::string& filename);

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
	ContextManager contextManager;
	BaseDemoContext baseDemoContext;

	const float mouseSensitivity;
	Camera cam;

private: //resources
	std::map<std::string, Mesh*> meshes;

private:
	void ClearFramebufferStack();

	void CoreDraw();

	//void ContextManagerMouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent);
	//void ContextManagerKeyPressed(sf::Event::KeyEvent key);
	//void ContextManagerKeyReleased(sf::Event::KeyEvent key);

	void Resize(const int width, const int height);
};
