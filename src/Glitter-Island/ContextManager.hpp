#pragma once

#include "GUIContext.hpp"
#include "EditorContext.hpp"
#include <SFML/Window.hpp>

class DemoCore;

class ContextManager
{
public:
	ContextManager(DemoCore* pCore = nullptr);
	~ContextManager();

	void Update(float deltaSec);
	void Draw();
	void DrawOverlayElements();
	void HandleWindowEvent(const sf::Event& event);

	GUIContext* GetTopContext();
	void PushContext(GUIContext* context);
	void PopContext();

	void PassEvent(GUIContext* sourceContext, const sf::Event& event);

private:
	//DemoCore* pCore;
	std::vector<GUIContext*> activeGUIStack;
};
