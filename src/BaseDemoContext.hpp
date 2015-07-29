#pragma once

#include "GUIContext.hpp"

class BaseDemoContext : public GUIContext
{
public:
	BaseDemoContext(DemoCore* pDemoCore);
	~BaseDemoContext();

	void MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent) override;
	void KeyPressed(sf::Event::KeyEvent key) override;
	void KeyReleased(sf::Event::KeyEvent key) override;

	void EnteringContext() override;
	void LeavingContext() override;

	void Update(float deltaTime) override;
	void Draw() override;
};
