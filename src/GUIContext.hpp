#pragma once

#include "all_gl_headers.hpp"
#include <SFML/Window.hpp>

class ContextManager;
class DemoCore;

class GUIContext
{
public:
	GUIContext(ContextManager* pContextManager, DemoCore* pDemoCore);
	virtual ~GUIContext();

	virtual void EnteringContext() = 0;
	virtual void LeavingContext() = 0;

	virtual void HandleWindowEvent(const sf::Event& event) = 0;

	virtual void Update(float deltaSec) = 0;
	virtual void Draw() = 0;
	virtual void DrawOverlayElements() = 0;

	void SetRequireFocus(bool value);
	bool GetRequireFocus() const;

protected:
	ContextManager* pContextManager;
	DemoCore* pDemoCore;
	bool requireFocus;
};
