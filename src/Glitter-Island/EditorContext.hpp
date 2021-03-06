#pragma once

#include "GUIContext.hpp"


//WARNING assuming that model selection won't include this file!
#include "ModelSelectionContext.hpp"
#include <GE/GraphicalObject.hpp>
#include <GE/Utility.hpp>
#include <GE/SimpleColoredMaterial.hpp>

class EditorContext : public GUIContext
{
public:
	enum class EditorTool { _FIRST = -1, NO_TOOL = 0, PAINT_FLAT_SAND, PAINT_SAND_TEXTURE, PAINT_GRASS_TEXTURE, SPAWN_GRASS_BUNCH, SPAWN_ROCK_BUNCH, PLACE_MODEL, _LAST };
	enum class EditorToolType { NO_TOOL, PAINT, SPAWN, PLACE };

	struct EditorToolInfo
	{
		std::int8_t id;
		std::string description;
	};

public:
	static EditorToolType GetToolType(EditorTool tool);
	static EditorToolInfo GetToolInfo(EditorTool tool);
	template<typename FuncType>
	static void ForEachTool(FuncType func)
	{
		for (int i = static_cast<int>(EditorTool::_FIRST) + 1; i < static_cast<int>(EditorTool::_LAST); i++) {
			func(static_cast<EditorTool>(i));
		}
	}

public:
	EditorContext(ContextManager* pContextManager, DemoCore* pCore);
	~EditorContext();

	void HandleWindowEvent(const sf::Event& event) override;

	void EnteringContext() override;
	void LeavingContext() override;

	void Update(float deltaSec) override;
	void Draw() override;
	void DrawOverlayElements() override;

private:
	//bool isInEditorMode;
	EditorTool selectedTool;
	glm::vec4 pointPosAtCursor;
	float brushRadius;
	Mesh brushCircleMesh;
	SimpleColoredMaterial brushCircleMaterial;
	util::managed_ptr<GraphicalObject> brushCircle;

	ModelSelectionContext modelSelectionContext;

private:
	void MouseButtonPressed(const sf::Event& event);
	void KeyPressed(const sf::Event& event);
	void UpdatePointPosAtCursor();
	void UpdateBrushCirclePos();
};
