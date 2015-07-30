#include "ContextManager.hpp"

ContextManager::ContextManager(DemoCore* pDemoCore)
{
}

ContextManager::~ContextManager()
{
}

void ContextManager::Update(float deltaSec)
{
	for (auto current : activeGUIStack) {
		current->Update(deltaSec);
	}
}

void ContextManager::Draw()
{
	for (auto current : activeGUIStack) {
		current->Draw();
	}
}

void ContextManager::DrawOverlayElements()
{
	for (auto current : activeGUIStack) {
		current->DrawOverlayElements();
	}
}

void ContextManager::HandleWindowEvent(const sf::Event& event)
{
	activeGUIStack.back()->HandleWindowEvent(event);
}

GUIContext* ContextManager::GetTopContext()
{
	return activeGUIStack.back();
}

void ContextManager::PushContext(GUIContext* context)
{
	activeGUIStack.push_back(context);
	context->EnteringContext();
}

void ContextManager::PopContext()
{
	activeGUIStack.back()->LeavingContext();
	activeGUIStack.pop_back();
}

void ContextManager::PassEvent(GUIContext* sourceContext, const sf::Event& event)
{
	auto iterator = std::find(activeGUIStack.begin(), activeGUIStack.end(), sourceContext);
	
	if (iterator != activeGUIStack.end() && iterator != activeGUIStack.begin()) {
		iterator--;
		(*iterator)->HandleWindowEvent(event);
	}
}
