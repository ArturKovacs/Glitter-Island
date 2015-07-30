#include "GUIContext.hpp"

#include "ContextManager.hpp"
#include "DemoCore.hpp"

GUIContext::GUIContext(ContextManager* pContextManager, DemoCore* pDemoCore) : pContextManager(pContextManager), pDemoCore(pDemoCore), requireFocus(false)
{}

GUIContext::~GUIContext()
{}

void GUIContext::SetRequireFocus(bool value)
{
	requireFocus = value;
}

bool GUIContext::GetRequireFocus() const
{
	return requireFocus;
}
