#pragma once

#include "DirectionalLight.hpp"
#include "Terrain.hpp"
#include "Skybox.hpp"
#include "Water.hpp"
#include "GraphicalObject.hpp"

#include "GUIContext.hpp"

//might not be a good design, but assuming that
//editor context wont contain an instance of BaseDemoContext.
#include "EditorContext.hpp"

class BaseDemoContext : public GUIContext
{
public:
	BaseDemoContext(ContextManager* pContextManager, DemoCore* pDemoCore);
	~BaseDemoContext();

	void HandleWindowEvent(const sf::Event& event) override;

	void EnteringContext() override;
	void LeavingContext() override;

	void Update(float deltaSec) override;
	void Draw() override;
	void DrawOverlayElements() override;

	float GetCurrentSpeed() const;
	bool GetWireframeModeEnabled() const;

	DirectionalLight& GetSun();
	Camera& GetCamera();
	Terrain& GetTerrain();

private:
	const float waterLevel;
	const float terrainSize;

	DirectionalLight sun;
	Terrain terrain;
	Skybox skybox;
	Water water;
	//Fog fog;

	std::vector<GraphicalObject> graphicalObjects;

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

private:
	EditorContext editorContext;

private:
	void KeyPressed(sf::Event::KeyEvent key);
	void KeyReleased(sf::Event::KeyEvent key);

	void DrawObjects();
};
