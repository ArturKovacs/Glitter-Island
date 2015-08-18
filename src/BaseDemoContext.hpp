#pragma once

#include "GUIContext.hpp"

#include "DirectionalLight.hpp"
#include "Terrain.hpp"
#include "Skybox.hpp"
#include "Water.hpp"
#include "GraphicalObject.hpp"
#include "DepthOnlyMaterial.hpp"
#include "PerspectiveCamera.hpp"
#include "RawCamera.hpp"
#include "Framebuffer.hpp"

//might not be a good design, but assuming that
//editor context wont contain an instance of BaseDemoContext.
#include "EditorContext.hpp"

class BaseDemoContext : public GUIContext
{
public:
	BaseDemoContext(ContextManager* pContextManager, DemoCore* pCore);
	~BaseDemoContext();

	void HandleWindowEvent(const sf::Event& event) override;

	void EnteringContext() override;
	void LeavingContext() override;

	void Update(float deltaSec) override;
	void Draw() override;
	void DrawOverlayElements() override;

	void AddGraphicalObject(GraphicalObject&& newObject);

	float GetCurrentSpeed() const;
	bool GetWireframeModeEnabled() const;

	DirectionalLight& GetSun();
	Camera& GetCamera();
	Terrain& GetTerrain();

	int GetLightCascadeCount() const;
	const gl::Texture& GetCascadeShadowMap(int cascadeID) const;
	gl::Mat4f GetCascadeViewProjectionTransform(int cascadeID) const;
	float GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const;

private:
	const float waterLevel;
	const float terrainSize;

	DirectionalLight sun;
	Terrain terrain;
	Skybox skybox;
	Water water;
	//Fog fog;

	std::vector<GraphicalObject> graphicalObjects;

	PerspectiveCamera debugCam;
	PerspectiveCamera cam;

	PerspectiveCamera* selectedCam;

private: //CSM
	DepthOnlyMaterial depthMaterial;

	static const int shadowMapResolution = 2048;
	static const int lightCascadeCount = 5;
	/// Position of the far planes of the subfrusta. Values mean a linear interpolation where 0 is view frustum's nearPlane, and 1 is view frustum's farPlane.
	/// The value with the lowest index corresponds to the nearest subfrustum.
	std::array<float, lightCascadeCount> subfrustumFarPlanePositionRatios;
	std::array<RawCamera, lightCascadeCount> lightCascadeCameras;
	std::array<Framebuffer, lightCascadeCount> lightCascadeShadowMapFramebuffers;

private: //user state
	enum class SpeedMode { NORMAL, FAST, ULTRA };

	const float camSpeed;
	const float fastSpeedMultiplyer;
	const float ultraSpeedMultiplyer;
	SpeedMode currentSpeedMode;

	bool wireframeModeEnabled;

	bool isTrackingMouse;
	float forwardMovement;
	float rightMovement;

	float sunAngleRad;

private:
	EditorContext editorContext;

private:
	void KeyPressed(sf::Event::KeyEvent key);
	void KeyReleased(sf::Event::KeyEvent key);

	void DrawObjects();
};
