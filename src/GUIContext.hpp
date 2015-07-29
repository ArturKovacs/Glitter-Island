#pragma once

#include "all_gl_headers.hpp"
#include <SFML/Window.hpp>

class DemoCore;

class GUIContext
{
public:
	GUIContext(DemoCore* pDemoCore);
	virtual ~GUIContext();

	virtual void EnteringContext() = 0;
	virtual void LeavingContext() = 0;

	virtual void MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent) = 0;
	virtual void KeyPressed(sf::Event::KeyEvent key) = 0;
	virtual void KeyReleased(sf::Event::KeyEvent key) = 0;

	virtual void Update(float deltaSec) = 0;
	virtual void Draw() = 0;
	virtual void DrawOverlayElements() = 0;

	void SetRequireFocus(bool value);
	bool GetRequireFocus() const;

protected:
	DemoCore* pDemoCore;
	bool requireFocus;
};
