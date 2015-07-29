#include "ModelSelectionContext.hpp"

ModelSelectionContext::ModelSelectionContext(DemoCore* pDemoCore) : GUIContext(pDemoCore)
{
	requireFocus = true;
}

ModelSelectionContext::~ModelSelectionContext()
{
}

void ModelSelectionContext::MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent)
{
}

void ModelSelectionContext::KeyPressed(sf::Event::KeyEvent key)
{
}

void ModelSelectionContext::KeyReleased(sf::Event::KeyEvent key)
{
}

void ModelSelectionContext::EnteringContext() 
{
}

void ModelSelectionContext::LeavingContext()
{
}

void ModelSelectionContext::Update(float deltaTime) 
{
}

void ModelSelectionContext::Draw() 
{
}
