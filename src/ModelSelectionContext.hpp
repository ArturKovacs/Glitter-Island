#pragma once

#include "GUIContext.hpp"

class ModelSelectionContext : public GUIContext
{
public:
	ModelSelectionContext(DemoCore* pDemoCore);
	~ModelSelectionContext();

	void MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent) override;
	void KeyPressed(sf::Event::KeyEvent key) override;
	void KeyReleased(sf::Event::KeyEvent key) override;

	void EnteringContext() override;
	void LeavingContext() override;

	void Update(float deltaTime) override;
	void Draw() override;
};
