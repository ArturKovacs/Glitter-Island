#pragma once

#include "GUIContext.hpp"

#include <GE/GraphicalObject.hpp>
#include <GE/PerspectiveCamera.hpp>
#include <GE/RawCamera.hpp>

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

	float GetCurrentSpeed() const;

	//Camera& GetCamera();

private: //user state
	enum class SpeedMode { NORMAL, FAST, ULTRA };

	const float camSpeed;
	const float fastSpeedMultiplyer;
	const float ultraSpeedMultiplyer;
	SpeedMode currentSpeedMode;

	bool isTrackingMouse;
	float forwardMovement;
	float rightMovement;

	float sunAngleRad;

private:
	PerspectiveCamera debugCam;
	PerspectiveCamera cam;

	PerspectiveCamera* selectedCam;
	
	EditorContext editorContext;

private:
	void KeyPressed(sf::Event::KeyEvent key);
	void KeyReleased(sf::Event::KeyEvent key);
};
