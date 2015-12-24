#pragma once

#include "all_gl_headers.hpp"

#include "Framebuffer.hpp"
#include "DefaultFramebuffer.hpp"
#include "DirectionalLight.hpp"
#include "Terrain.hpp"
#include "Skybox.hpp"
#include "Water.hpp"
#include "Camera.hpp"
#include "RawCamera.hpp"
#include "DebugDrawer.hpp"
#include "SimpleColoredDrawer.hpp"

#include "Utility.hpp"

#include "../../implementation/StandardGraphicalObject.hpp"
#include "../../implementation/MeshManager.hpp"
#include "../../implementation/MaterialManager.hpp"

#include <string>
#include <deque>
#include <unordered_set>

class GraphicsEngine
{
public:
	static std::string& GetShadersFolderPath();
	static std::string& GetImgFolderPath();
	static std::string& GetModelsFolderPath();
	
	static gl::Program LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name);
	
public:
	GraphicsEngine();
	
	void Update(double elapsedSeconds);
	void Draw();

	double GetElapsedSeconds() const;
	
	void Resize(const int width, const int height);
	
	int GetScreenWidth() const;
	int GetScreenHeight() const;
	
	void SetCurrentFramebuffer(Framebuffer& current);
	Framebuffer& GetCurrentFramebuffer();
	Framebuffer& GetIntermediateFramebuffer();
	void AddFramebufferForManagment(Framebuffer& framebuffer);
	
	float GetObjectsDepthBufferValue(int x, int y);
	
	gl::Context& GetGLContext();
	
	void SetWireframeModeEnabled(bool enabled);
	bool GetWireframeModeEnabled() const;
	
	DirectionalLight& GetSun();
	Terrain& GetTerrain();
	Water& GetWater();
	
	void SetActiveCamera(Camera* cam);
	Camera* GetActiveCamera();
	void SetActiveViewerCamera(PerspectiveCamera* cam);
	
	int GetLightCascadeCount() const;
	const gl::Texture& GetCascadeShadowMap(int cascadeID) const;
	glm::mat4 GetCascadeViewProjectionTransform(int cascadeID) const;
	float GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const;
	
	util::managed_ptr<Mesh> LoadMeshFromFile(const std::string& filename);
	util::managed_ptr<Material> LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName);
	util::managed_ptr<GraphicalObject> CreateGraphicalObject();
	
	DebugDrawer& GetDebugDrawer();
	SimpleColoredDrawer& GetSimpleColoredDrawer();
	
private:
	static std::string shadersFolderPath;
	static std::string imgFolderPath;
	static std::string modelsFolderPath;
	
private: //misc
	double elapsedSec; //Seconds elapsed from an arbitrary moment in the past to present. (The actual value does not matter, only requirement is to increase continously, and one unit by a second)

private: // graphical state
	int screenWidth;
	int screenHeight;
	
	gl::Context glContext;
	
	Framebuffer aoResultFB;
	Framebuffer objectsFB;
	Framebuffer halfSizedIntermFramebuffer;
	Framebuffer intermediateFramebuffer;
	Framebuffer* pCurrentFramebuffer;
	std::vector<Framebuffer*> managedFramebuffers;
	gl::DefaultFramebuffer defaultFBO;

	gl::Program finalFramebufferCopy;
	gl::Uniform<GLint> framebufferCopy_ScreenWidth;
	gl::Uniform<GLint> framebufferCopy_ScreenHeight;
	
	bool wireframeModeEnabled;
	
private: // objects
	const float waterLevel;
	const float terrainSize;

	DirectionalLight sun;
	Terrain terrain;
	Skybox skybox;
	Water water;
	//Fog fog;

	//it is a deque so that all Graphical Object pointers remain valid after a new Graphical Object is inserted
	std::deque<StandardGraphicalObject> managedGraphicalObjects;
	std::map<Mesh::Submesh*, std::unordered_set<GraphicalObject*>> instancedGraphicalObjects;
	
	Camera* pActiveCam;
	PerspectiveCamera* pActiveViewerCam;

	DebugDrawer debugDrawer;
	SimpleColoredDrawer simpleColoredDrawer;
	
private: //CSM
	static constexpr int shadowMapResX = 2048;
	static constexpr int shadowMapResY = 1024;
	static constexpr int lightCascadeCount = 5;
	
	/// Position of the far planes of the subfrusta. Values mean a linear interpolation where 0 is view frustum's nearPlane, and 1 is view frustum's farPlane.
	/// The value with the lowest index corresponds to the nearest subfrustum.
	std::array<float, lightCascadeCount> subfrustumFarPlanePositionRatios;
	std::array<RawCamera, lightCascadeCount> lightCascadeCameras;
	std::array<Framebuffer, lightCascadeCount> lightCascadeShadowMapFramebuffers;

	glm::mat4 lightViewTransform;
	
private: //Ambient Occlusion
	gl::Program ssaoCalcProgram;
	gl::Uniform<glm::mat4> ssaoCalc_viewProj;
	gl::Uniform<glm::mat4> ssaoCalc_viewProjInv;
	gl::Uniform<GLint> ssaoCalc_screenWidth;
	gl::Uniform<GLint> ssaoCalc_screenHeight;

	gl::Program ssaoDrawProgram;
	gl::Uniform<GLint> ssaoDraw_screenWidth;
	gl::Uniform<GLint> ssaoDraw_screenHeight;

private: //resources
	MeshManager meshManager;
	MaterialManager materialManager;
	
private:
	
	void UpdateLightViewTransform();
	float GetSubfrustumZNear(int cascadeID) const;
	void UpdateLightCascadeCamera(int cascadeID);
	void DrawShadowMap(int cascadeID);
	void DrawObjects();
	void DrawScene();
	void DrawAmbientOcclusion();
};
