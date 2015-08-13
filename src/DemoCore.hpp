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
#include "MeshManager.hpp"
#include "MaterialManager.hpp"
#include "DebugDrawer.hpp"

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

	int GetScreenWidth() const;
	int GetScreenHeight() const;

	void SetActiveCamera(Camera* cam);
	Camera* GetActiveCamera();

	DebugDrawer& GetDebugDrawer();

	DirectionalLight& GetSun();
	Terrain& GetTerrain();
	float GetMouseSensitivity();

	sf::Window* GetWindow();

	int GetLightCascadeCount() const;
	const gl::Texture& GetCascadeShadowMap(int cascadeID) const;
	gl::Mat4f GetCascadeViewProjectionTransform(int cascadeID) const;
	float GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const;

	void AddGraphicalObject(GraphicalObject&& newObject);

	Mesh* LoadMeshFromFile(const std::string& filename);
	Material* LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName);
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
	Camera* pActiveCam;

	DebugDrawer debugDrawer;

private: //resources
	MeshManager meshManager;
	MaterialManager materialManager;

private:
	void ClearFramebufferStack();

	void CoreDraw();

	void Resize(const int width, const int height);
};
