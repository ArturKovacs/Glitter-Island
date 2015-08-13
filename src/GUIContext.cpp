#include "GUIContext.hpp"

#include "ContextManager.hpp"
#include "DemoCore.hpp"

GUIContext::GUIContext(ContextManager* pContextManager, DemoCore* pCore) : pContextManager(pContextManager), pCore(pCore), requireFocus(false)
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
