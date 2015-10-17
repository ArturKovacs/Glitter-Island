#include "EditorContext.hpp"

#include "DemoCore.hpp"
#include "Utility.hpp"
#include <GE/SimpleColoredMaterial.hpp>

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

	return EditorToolType::NO_TOOL;
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

EditorContext::EditorContext(ContextManager* pContextManager, DemoCore* pCore) :
GUIContext(pContextManager, pCore),
modelSelectionContext(pContextManager, pCore),
selectedTool(EditorTool::NO_TOOL), brushRadius(1),
brushCircleMaterial(&(pCore->GetGraphicsEngine()))
{
	modelSelectionContext.ForceUpdateModelFileList();

	brushCircleMesh = Mesh::GenerateCircle(1, 16);
	assert(brushCircleMesh.GetSubmeshes().size() == 1);
	brushCircleMaterial.SetColor(gl::Vec4f(1, 0.1, 0.5, 1));
	brushCircleMesh.GetSubmeshes().at(0).SetMaterial(&brushCircleMaterial);
	
	brushCircle.SetMesh(&brushCircleMesh);
	brushCircle.SetDepthTestEnabled(false);
	brushCircle.SetVisible(false);
	
	pCore->GetGraphicsEngine().AddGraphicalObject(&brushCircle);
}

EditorContext::~EditorContext()
{}

void EditorContext::HandleWindowEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseWheelMoved) {
		brushRadius += -event.mouseWheel.delta*0.5;
		brushRadius = std::max(brushRadius, 0.5f);
	}
	else if (event.type == sf::Event::KeyPressed) {
		KeyPressed(event);
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		MouseButtonPressed(event);
	}
	else {
		pContextManager->PassEvent(this, event);
	}
}

void EditorContext::EnteringContext()
{
	//pCore->GetGraphicsEngine().GetDebugDrawer().SetEnabled(true);
	//brushCircle.SetVisible(true);
}

void EditorContext::LeavingContext()
{
	//pCore->GetGraphicsEngine().GetDebugDrawer().SetEnabled(false);
	brushCircle.SetVisible(false);
}

void EditorContext::Update(float deltaSec)
{
	UpdatePointPosAtCursor();

	brushCircle.SetVisible(GetToolType(selectedTool) == EditorToolType::PAINT);
	
	if (GetToolType(selectedTool) == EditorToolType::PAINT) {
		UpdateBrushCirclePos();
	}
	
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		switch (GetToolType(selectedTool)) {
		case EditorToolType::PAINT: {
			
			gl::Vec2i cursorPosOnMaterialMap = pCore->GetGraphicsEngine().GetTerrain().GetMaterialMapPos(pointPosAtCursor);
			sf::Image& materialMap = pCore->GetGraphicsEngine().GetTerrain().GetMaterialMap();
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

			pCore->GetGraphicsEngine().GetTerrain().DownloadMaterialMapToGPU();
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
	assert(false);
}

void EditorContext::DrawOverlayElements()
{
	gl::Context& glContext = pCore->GetGraphicsEngine().GetGLContext();

	//TODO move overlay rendering to Core! Other obejcts should only be able to add overlay elements to core - review this statement!! -

	sf::Text text("Editor mode", pCore->overlayFont);
	text.setPosition(sf::Vector2f(30, 25));
	text.setCharacterSize(18);
	text.setColor(sf::Color(210, 65, 240, ~sf::Uint8(0)));
	text.setStyle(sf::Text::Bold);
	pCore->textDrawer.DrawBackground(glContext, text, sf::Color(50, 50, 50, 150), 5);
	pCore->textDrawer.Draw(glContext, text);

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
	text.setColor(sf::Color::White);
	pCore->textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 150), 5);
	pCore->textDrawer.DrawAsList(glContext, text, GetToolInfo(selectedTool).id+1, sf::Color::Cyan);

	if (selectedTool == EditorTool::PLACE_MODEL) {
		str = "Selected model:\n";
		if (modelSelectionContext.GetModelFileCount() > 0) {
			str += modelSelectionContext.GetSelectedModelFilename();
		}
		else {
			str += "## NO MODELS FOUND ##";
		}
		text.setString(str);
		text.setPosition(sf::Vector2f(2, 350));
		pCore->textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 150), 5);
		pCore->textDrawer.Draw(glContext, text);
	}
}

void EditorContext::MouseButtonPressed(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Button::Left) {

		if (selectedTool == EditorTool::PLACE_MODEL) {
			static float TMP_rot = 0;
			TMP_rot += (double(std::rand())/RAND_MAX)*gl::math::Pi() + gl::math::Pi()*0.5;
			GraphicalObject loadedObject = pCore->GetGraphicsEngine().LoadGraphicalObjectFromFile(modelSelectionContext.GetSelectedModelFilename());
			loadedObject.SetTransform(gl::ModelMatrixf::Translation(0, -0.1, 0) * gl::ModelMatrixf::Translation(pointPosAtCursor) * gl::ModelMatrixf::RotationY(gl::Radians<float>(TMP_rot)));

			pCore->GetGraphicsEngine().AddGraphicalObject(std::move(loadedObject));
		}
	}
	else {
		pContextManager->PassEvent(this, event);
	}
}

void EditorContext::KeyPressed(const sf::Event& event)
{
	const sf::Event::KeyEvent& key = event.key;

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
		//pContextManager->PushContext(&modelSelectionContext);
		break;
	case sf::Keyboard::Add:
		brushRadius += 4.0f;
		break;
	case sf::Keyboard::Subtract:
		brushRadius -= 4.0f;
		brushRadius = std::max(brushRadius, 0.5f);
		break;
	case sf::Keyboard::R:
		selectedTool = static_cast<EditorTool>(static_cast<int>(selectedTool) - 1);
		selectedTool = selectedTool == EditorTool::_FIRST ? static_cast<EditorTool>(static_cast<int>(EditorTool::_LAST) - 1) : selectedTool;
		break;

	case sf::Keyboard::F:
		selectedTool = static_cast<EditorTool>(static_cast<int>(selectedTool) + 1);
		selectedTool = selectedTool == EditorTool::_LAST ? static_cast<EditorTool>(static_cast<int>(EditorTool::_FIRST) + 1) : selectedTool;
		break;

	case sf::Keyboard::M:
		pCore->SaveAll();
		break;

	case sf::Keyboard::E:
		if (pContextManager->GetTopContext() == this) {
			pContextManager->PopContext();
		}
		break;

	case sf::Keyboard::Space:
		if (selectedTool == EditorTool::PLACE_MODEL) {
			pContextManager->PushContext(&modelSelectionContext);
		}
		break;

	default:
		pContextManager->PassEvent(this, event);
		break;
	}
}

void EditorContext::UpdatePointPosAtCursor()
{
	int screenWidth = pCore->GetScreenWidth();
	int screenHeight = pCore->GetScreenHeight();

	sf::Vector2i cursorPos = sf::Mouse::getPosition(*pCore->GetWindow());

	//invert y to suit opengl coordinates
	cursorPos.y = screenHeight-cursorPos.y;

	GLfloat depthAtPixel;
	pCore->GetGraphicsEngine().GetGLContext().ReadPixels(cursorPos.x, cursorPos.y, 1, 1, gl::enums::PixelDataFormat::DepthComponent, gl::PixelDataType::Float, &depthAtPixel);

	pointPosAtCursor = gl::Inverse(pCore->GetGraphicsEngine().GetActiveCamera()->GetViewProjectionTransform()) * gl::Vec4f(
		((float(cursorPos.x)/screenWidth)*2-1),
		((float(cursorPos.y)/screenHeight)*2-1),
		(depthAtPixel)*2-1,
		1);

	pointPosAtCursor = pointPosAtCursor / pointPosAtCursor.w();
}

void EditorContext::UpdateBrushCirclePos()
{
	float meshScale = pCore->GetGraphicsEngine().GetTerrain().GetMaterialMapPixelSizeInWorldScale() * brushRadius;
	gl::Mat4f modelMatrix =
		gl::ModelMatrixf::Translation(pointPosAtCursor.xyz()) *
		gl::ModelMatrixf::Scale(meshScale, meshScale, meshScale) *
		gl::ModelMatrixf::RotationX(gl::Radians<float>(gl::math::Pi()*0.5));

	brushCircle.SetTransform(modelMatrix);
}
