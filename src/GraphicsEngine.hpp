#pragma once

#include "all_gl_headers.hpp"

#include "Framebuffer.hpp"
#include "DefaultFramebuffer.hpp"
#include "GraphicalObject.hpp"
#include "DirectionalLight.hpp"
#include "Terrain.hpp"
#include "Skybox.hpp"
#include "Water.hpp"
#include "Camera.hpp"
#include "RawCamera.hpp"
#include "DebugDrawer.hpp"
#include "SimpleColoredDrawer.hpp"
#include "ContextManager.hpp"
#include "MeshManager.hpp"
#include "MaterialManager.hpp"

#include <string>
#include <list>

class DemoCore;

class GraphicsEngine
{
public:
	static std::string& GetShadersFolderPath();
	static std::string& GetImgFolderPath();
	static std::string& GetModelsFolderPath();
	
	static gl::Program LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name);
	
public:
	GraphicsEngine(ContextManager *pContextManager);
	
	void Draw(DemoCore* pCore);
	
	void Resize(const int width, const int height);
	
	//void SetScreenWidth(int width);
	//void SetScreenHeight(int height);
	
	int GetScreenWidth() const;
	int GetScreenHeight() const;
	
	void PushFramebuffer();
	void PopFramebuffer();

	void CopyFramebufferContents(const Framebuffer& source);

	Framebuffer& GetCurrentFramebuffer();
	gl::Context& GetGLContext();
	
	void SetWireframeModeEnabled(bool enabled);
	bool GetWireframeModeEnabled() const;
	
	DirectionalLight& GetSun();
	Terrain& GetTerrain();
	
	void SetActiveCamera(Camera* cam);
	Camera* GetActiveCamera();
	void SetActiveViewerCamera(PerspectiveCamera* cam);
	
	int GetLightCascadeCount() const;
	const gl::Texture& GetCascadeShadowMap(int cascadeID) const;
	gl::Mat4f GetCascadeViewProjectionTransform(int cascadeID) const;
	float GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const;
	
	void AddGraphicalObject(GraphicalObject&& newObject);
	void AddGraphicalObject(GraphicalObject* newObject);
	
	Mesh* LoadMeshFromFile(const std::string& filename);
	Material* LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName);
	GraphicalObject LoadGraphicalObjectFromFile(const std::string& filename);
	
	DebugDrawer& GetDebugDrawer();
	SimpleColoredDrawer& GetSimpleColoredDrawer();
	
private:
	static std::string shadersFolderPath;
	static std::string imgFolderPath;
	static std::string modelsFolderPath;
	
private:
	int screenWidth;
	int screenHeight;
	
	gl::Context glContext;

	std::list<Framebuffer> framebuffers;
	gl::DefaultFramebuffer defaultFBO;

	gl::Program finalFramebufferCopy;
	gl::Uniform<GLint> framebufferCopy_ScreenWidth;
	gl::Uniform<GLint> framebufferCopy_ScreenHeight;
	
	bool wireframeModeEnabled;
	
private:
	const float waterLevel;
	const float terrainSize;

	DirectionalLight sun;
	Terrain terrain;
	Skybox skybox;
	Water water;
	//Fog fog;

	std::vector<GraphicalObject> managedGraphicalObjects;
	std::vector<GraphicalObject*> externalGraphicalObjects;
	
	Camera* pActiveCam;
	PerspectiveCamera* pActiveViewerCam;

	DebugDrawer debugDrawer;
	SimpleColoredDrawer simpleColoredDrawer;
	
private: //CSM
	//DepthOnlyMaterial depthMaterial;

	static const int shadowMapResolution = 2048;
	static const int lightCascadeCount = 5;
	
	/// Position of the far planes of the subfrusta. Values mean a linear interpolation where 0 is view frustum's nearPlane, and 1 is view frustum's farPlane.
	/// The value with the lowest index corresponds to the nearest subfrustum.
	std::array<float, lightCascadeCount> subfrustumFarPlanePositionRatios;
	std::array<RawCamera, lightCascadeCount> lightCascadeCameras;
	std::array<Framebuffer, lightCascadeCount> lightCascadeShadowMapFramebuffers;

	gl::Mat4f lightViewTransform;
	
private: //resources
	
	ContextManager *pContextManager;
	MeshManager meshManager;
	MaterialManager materialManager;
	
private:
	void ClearFramebufferStack();
	
	void UpdateLightViewTransform();
	float GetSubfrustumZNear(int cascadeID) const;
	void UpdateLightCascadeCamera(int cascadeID);
	void DrawShadowMap(int cascadeID);
	void DrawObjects();
	void DrawScene(DemoCore* pCore);
};
