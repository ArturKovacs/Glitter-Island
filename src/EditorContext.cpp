#include "EditorContext.hpp"

#include "DemoCore.hpp"
#include "Utility.hpp"

EditorContext::EditorToolType EditorContext::GetToolType(EditorContext::EditorTool tool)
{
	switch (tool) {
	case EditorTool::PAINT_FLAT_SAND:
	case EditorTool::PAINT_SAND_TEXTURE:
	case EditorTool::PAINT_GRASS_TEXTURE:
		return EditorToolType::PAINT;
		break;

	case EditorTool::SPAWN_GRASS_BUNCH:
	case EditorTool::SPAWN_ROCK_BUNCH:
		return EditorToolType::SPAWN;
		break;

	case EditorTool::PLACE_MODEL:
		return EditorToolType::PLACE;
		break;

	case EditorTool::NO_TOOL:
		return EditorToolType::NO_TOOL;
		break;

	default:
		assert(false);
		break;
	}
}

EditorContext::EditorToolInfo EditorContext::GetToolInfo(EditorContext::EditorTool tool)
{
	EditorToolInfo result;

	result.id = static_cast<std::int8_t>(tool);

	switch (tool) {
	case EditorTool::PAINT_FLAT_SAND:
		result.description = "Paint flat sand";
		break;
	case EditorTool::PAINT_SAND_TEXTURE:
		result.description = "Paint sand texture";
		break;
	case EditorTool::PAINT_GRASS_TEXTURE:
		result.description = "Paint grass texture";
		break;
	case EditorTool::SPAWN_GRASS_BUNCH:
		result.description = "Spawn grass bunch";
		break;
	case EditorTool::SPAWN_ROCK_BUNCH:
		result.description = "Spawn rock bunch";
		break;
	case EditorTool::PLACE_MODEL:
		result.description = "Place model";
		break;
	case EditorTool::NO_TOOL:
		result.description = "Select no tool";
		break;

	default:
		assert(false);
		break;
	}

	return result;
}

EditorContext::EditorContext(DemoCore* pDemoCore) : 
GUIContext(pDemoCore),
selectedTool(EditorTool::NO_TOOL), brushRadius(1), showModelSelection(false), howeredModelID(0), selectedModelID(0)
{}

EditorContext::~EditorContext()
{}

void EditorContext::MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent)
{
	brushRadius += -wheelEvent.delta*0.5;
	brushRadius = std::max(brushRadius, 0.5f);
}

void EditorContext::KeyPressed(sf::Event::KeyEvent key)
{
	switch (key.code) {
	case sf::Keyboard::Num0:
		selectedTool = EditorTool::NO_TOOL;
		break;
	case sf::Keyboard::Num1:
		selectedTool = EditorTool::PAINT_FLAT_SAND;
		break;
	case sf::Keyboard::Num2:
		selectedTool = EditorTool::PAINT_SAND_TEXTURE;
		break;
	case sf::Keyboard::Num3:
		selectedTool = EditorTool::PAINT_GRASS_TEXTURE;
		break;
	case sf::Keyboard::Num4:
		selectedTool = EditorTool::SPAWN_GRASS_BUNCH;
		break;
	case sf::Keyboard::Num5:
		selectedTool = EditorTool::SPAWN_ROCK_BUNCH;
		break;
	case sf::Keyboard::Num6:
		selectedTool = EditorTool::PLACE_MODEL;
		showModelSelection = true;
		howeredModelID = selectedModelID;
		UpdateModelFileList();
		break;
	case sf::Keyboard::Add:
		brushRadius += 4.0f;
		break;
	case sf::Keyboard::Subtract:
		brushRadius -= 4.0f;
		brushRadius = std::max(brushRadius, 0.5f);
		break;

	case sf::Keyboard::M:
		pDemoCore->SaveAll();
		break;

	default:
		break;
	}

	if (showModelSelection) {
		switch (key.code) {
		case sf::Keyboard::Up:
			howeredModelID--;
			howeredModelID = std::max(howeredModelID, 0);
			break;
		case sf::Keyboard::Down:
			howeredModelID++;
			howeredModelID = std::min<int>(howeredModelID, modelFileList.size()-1);
			break;
		case sf::Keyboard::Return:
			showModelSelection = false;
			selectedModelID = howeredModelID;
			break;
		default:
			break;
		}
	}
}

void EditorContext::KeyReleased(sf::Event::KeyEvent key)
{}

void EditorContext::EnteringContext() 
{
}

void EditorContext::LeavingContext()
{
}

void EditorContext::Update(float deltaSec) 
{
	UpdatePointPosAtCursor();

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

		switch (GetToolType(selectedTool)) {
		case EditorToolType::PAINT: {
			gl::Vec2i cursorPosOnMaterialMap = pDemoCore->GetTerrain().GetMaterialMapPos(pointPosAtCursor);
			sf::Image& materialMap = pDemoCore->GetTerrain().GetMaterialMap();
			sf::Color selectedMaterialColor;
			if (selectedTool == EditorTool::PAINT_FLAT_SAND) {
				selectedMaterialColor = sf::Color(0, 0, 0, 255);
			}
			else if(selectedTool == EditorTool::PAINT_SAND_TEXTURE) {
				selectedMaterialColor = sf::Color(0, 0, 255, 255);
			}
			else if(selectedTool == EditorTool::PAINT_GRASS_TEXTURE) {
				selectedMaterialColor = sf::Color(0, 255, 0, 255);
			}

			const gl::Vec2f centerPos(cursorPosOnMaterialMap.x(), cursorPosOnMaterialMap.y());

			auto weightFunc = [](float x_sq, float radius_sq){
				return 1-(std::sqrt(x_sq)/std::sqrt(radius_sq));
			};

			for (int dy = -brushRadius; dy <= brushRadius; dy++) {
				for (int dx = -brushRadius; dx <= brushRadius; dx++) {
					gl::Vec2i currPosi(cursorPosOnMaterialMap.x() + dx, cursorPosOnMaterialMap.y() + dy);

					if (currPosi.x() >= 0 && currPosi.x() < materialMap.getSize().x &&
						currPosi.y() >= 0 && currPosi.y() < materialMap.getSize().y) {

						const gl::Vec2f currPos(currPosi.x(), currPosi.y());
						const gl::Vec2f posDiff = currPos - centerPos;
						float weight = weightFunc(gl::Dot(posDiff, posDiff), brushRadius*brushRadius) * (1 - std::exp(-10.f*(deltaSec)));

						if (weight > 0) {
							const sf::Color originalColor = materialMap.getPixel(currPos.x(), currPos.y());
							sf::Color finalColor(
								selectedMaterialColor.r*weight + originalColor.r*(1 - weight),
								selectedMaterialColor.g*weight + originalColor.g*(1 - weight),
								selectedMaterialColor.b*weight + originalColor.b*(1 - weight));

							materialMap.setPixel(currPosi.x(), currPosi.y(), finalColor);
						}
					}
				}
			}

			pDemoCore->GetTerrain().DownloadMaterialMapToGPU();
			break;
		}
		case EditorToolType::SPAWN: {
			break;
		}
		case EditorToolType::PLACE: {
			break;
		}
		case EditorToolType::NO_TOOL: {
			break;
		}
		default:
			assert(false);
			break;
		}
	}
}

void EditorContext::Draw() 
{
	gl::Context& glContext = pDemoCore->GetGLContext();

	EditorToolType selectedToolType = GetToolType(selectedTool);
	if (selectedToolType == EditorToolType::PAINT) {
		float meshScale = pDemoCore->GetTerrain().GetMaterialMapPixelSizeInWorldScale() * brushRadius;
		using ModelMatf = gl::ModelMatrixf;
		gl::Mat4f MVP =
			pDemoCore->GetCamera().GetViewProjectionTransform() *
			ModelMatf::Translation(pointPosAtCursor.xyz()) *
			ModelMatf::Scale(meshScale, meshScale, meshScale) *
			//ModelMatf::RotationY(gl::Radians<float>(gl::math::Pi()*0.25)) *
			ModelMatf::RotationX(gl::Radians<float>(gl::math::Pi()*0.5));

		//glContext.Disable(gl::Capability::CullFace);
		glContext.Disable(gl::Capability::DepthTest);
		glContext.LineWidth(2);
		pDemoCore->simpleColoredDrawer.Draw(glContext, pDemoCore->circle, MVP, gl::Vec4f(1, 0.1, 0.5, 1));
		glContext.LineWidth(1);
		glContext.Enable(gl::Capability::DepthTest);
		//glContext.Enable(gl::Capability::CullFace);
	}
}

void EditorContext::DrawOverlayElements()
{
	gl::Context& glContext = pDemoCore->GetGLContext();

	//TODO move overlay rendering to Core! Other obejcts should only be able to add overlay elements to core - review this statement!! -
	//glContext.Clear().DepthBuffer();
	glContext.Disable(gl::Capability::DepthTest);
	glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	glContext.Enable(gl::Capability::Blend);
	glContext.BlendFunc(gl::BlendFunction::SrcAlpha, gl::BlendFunction::OneMinusSrcAlpha);

	sf::Text text("-Editor mode-", pDemoCore->overlayFont);
	text.setPosition(sf::Vector2f(30, 25));
	text.setCharacterSize(18);
	text.setColor(sf::Color(208, 59, 237, ~sf::Uint8(0)));
	text.setStyle(sf::Text::Bold);
	pDemoCore->textDrawer.DrawBackground(glContext, text, sf::Color(50, 50, 50, 150), 5);
	pDemoCore->textDrawer.Draw(glContext, text);
		
	//Set complete text for calculating background correctly
	sf::String str = "E -- Toggle editor mode";
	ForEachTool([&](EditorTool current){
		EditorToolInfo info = GetToolInfo(current);
		str += std::string("\n") + std::to_string(info.id) + " -- " + info.description;
	});
	text.setString(str);
	text.setPosition(sf::Vector2f(2, 100));
	text.setCharacterSize(16);
	text.setStyle(sf::Text::Regular);
	text.setColor(sf::Color::Yellow);
	pDemoCore->textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 150), 5);
	pDemoCore->textDrawer.DrawAsList(glContext, text, GetToolInfo(selectedTool).id+1, sf::Color::Cyan);

	if (showModelSelection) {
		const int columnElementCount = 15;

		std::string visibleList;

		const int listFirstElementID = howeredModelID - howeredModelID % columnElementCount;

		for (int i = 0; i < columnElementCount && (listFirstElementID + i < modelFileList.size()); i++) {
			visibleList += modelFileList.at(listFirstElementID + i) + '\n';
		}
		visibleList.pop_back();

		text.setString(visibleList);
		text.setPosition(300, 50);
		pDemoCore->textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 150), 5);
		pDemoCore->textDrawer.DrawAsList(glContext, text, howeredModelID-listFirstElementID, sf::Color::Cyan);
	}

	glContext.Disable(gl::Capability::Blend);
}

void EditorContext::UpdateModelFileList()
{
	// Find all model files in models directory
	auto tmpList = util::GetFileNamesInDirectory("../models");

	modelFileList.clear();

	// only keep supported model files!
	for (int i = 0; i < tmpList.size(); i++) {
		const auto& current = tmpList[i];
		const std::string extention = ".obj";
		const bool correctExtension = current.rfind(extention) == current.length() - extention.length();
		if (correctExtension) {
			modelFileList.push_back(current);
		}
	}
}

void EditorContext::UpdatePointPosAtCursor()
{
	int screenWidth = pDemoCore->GetCamera().GetScreenWidth();
	int screenHeight = pDemoCore->GetCamera().GetScreenHeight();

	sf::Vector2i cursorPos = sf::Mouse::getPosition(*pDemoCore->GetWindow());

	//invert y to suit opengl coordinates
	cursorPos.y = screenHeight-cursorPos.y;

	GLfloat depthAtPixel;
	pDemoCore->GetGLContext().ReadPixels(cursorPos.x, cursorPos.y, 1, 1, gl::enums::PixelDataFormat::DepthComponent, gl::PixelDataType::Float, &depthAtPixel);

	pointPosAtCursor = gl::Inverse(pDemoCore->GetCamera().GetViewProjectionTransform()) * gl::Vec4f(
		((float(cursorPos.x)/screenWidth)*2-1),
		((float(cursorPos.y)/screenHeight)*2-1),
		(depthAtPixel)*2-1,
		1);

	pointPosAtCursor = pointPosAtCursor / pointPosAtCursor.w();
}
