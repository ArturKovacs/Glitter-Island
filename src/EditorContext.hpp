#pragma once

#include "GUIContext.hpp"

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
	EditorContext(DemoCore* pDemoCore);
	~EditorContext();

	void MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent) override;
	void KeyPressed(sf::Event::KeyEvent key) override;
	void KeyReleased(sf::Event::KeyEvent key) override;

	void EnteringContext() override;
	void LeavingContext() override;

	void Update(float deltaSec) override;
	void Draw() override;
	void DrawOverlayElements() override;

private:
	//bool isInEditorMode;
	EditorTool selectedTool;
	gl::Vec4f pointPosAtCursor;
	float brushRadius;
	bool showModelSelection;
	int howeredModelID;
	int selectedModelID;
	std::vector<std::string> modelFileList;

private:
	void UpdateModelFileList();
	void UpdatePointPosAtCursor();
};
