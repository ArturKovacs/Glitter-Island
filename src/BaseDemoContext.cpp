#include "BaseDemoContext.hpp"

BaseDemoContext::BaseDemoContext(DemoCore* pDemoCore) : GUIContext(pDemoCore)
{}

BaseDemoContext::~BaseDemoContext()
{}

void BaseDemoContext::MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent)
{
}

void BaseDemoContext::KeyPressed(sf::Event::KeyEvent key)
{
}

void BaseDemoContext::KeyReleased(sf::Event::KeyEvent key)
{
}

void BaseDemoContext::EnteringContext() 
{
}

void BaseDemoContext::LeavingContext()
{
}

void BaseDemoContext::Update(float deltaTime) 
{
}

void BaseDemoContext::Draw() 
{
}
