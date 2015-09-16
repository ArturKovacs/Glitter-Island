#pragma once

#include "GUIContext.hpp"

#include <GE/Mesh.hpp>

class ModelSelectionContext : public GUIContext
{
public:
	ModelSelectionContext(ContextManager* pContextManager, DemoCore* pCore);
	~ModelSelectionContext();

	void HandleWindowEvent(const sf::Event& event) override;

	void EnteringContext() override;
	void LeavingContext() override;

	void Update(float deltaSec) override;
	void Draw() override;
	void DrawOverlayElements() override;

	std::string GetSelectedModelFilename() const;
	int GetModelFileCount();
	void ForceUpdateModelFileList();

private:
	Mesh screenRectangle;

	int howeredModelID;
	int selectedModelID;
	std::vector<std::string> modelFileList;

private:
	void UpdateModelFileList();
};
