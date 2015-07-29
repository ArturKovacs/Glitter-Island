#include "GUIContext.hpp"

#include "DemoCore.hpp"

GUIContext::GUIContext(DemoCore* pDemoCore) : pDemoCore(pDemoCore), requireFocus(false)
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
