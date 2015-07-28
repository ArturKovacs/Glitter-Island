#include "DemoCore.hpp"

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "Utility.hpp"

const std::string DemoCore::shadersFolderPath = "../shaders/";
const std::string DemoCore::imgFolderPath = "../img/";
const std::string DemoCore::modelsFolderPath = "../models/";

DemoCore::EditorToolType DemoCore::GetToolType(DemoCore::EditorTool tool)
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

DemoCore::EditorToolInfo DemoCore::GetToolInfo(DemoCore::EditorTool tool)
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

gl::Program DemoCore::LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name)
{
	gl::Program result;

	gl::VertexShader vs;
	gl::GLSLSource vs_src((shadersFolderPath + vs_name).c_str(), gl::GLSLSource::FromFile_());
	vs.Source(vs_src);
	//vs.Source(LoadFileAsString(shadersFolderPath + vs_name));
	vs.Compile();

	gl::FragmentShader fs;
	gl::GLSLSource fs_src((shadersFolderPath + fs_name).c_str(), gl::GLSLSource::FromFile_());
	fs.Source(fs_src);
	//fs.Source(LoadFileAsString(shadersFolderPath + fs_name));
	fs.Compile();

	result.AttachShader(vs);
	result.AttachShader(fs);
	result.Link();

	return result;
}

DemoCore::DemoCore(sf::Window* pWindow) :
running(false),
isInEditorMode(false), selectedTool(EditorTool::NO_TOOL), brushRadius(1), showModelSelection(false), howeredModelID(0), selectedModelID(0),
pWindow(pWindow),
mouseSensitivity(0.005), camSpeed(3), fastSpeedMultiplyer(8.5), ultraSpeedMultiplyer(30),
terrainSize(500), waterLevel(49), water(terrainSize * 7)
{
	//assert(pWindow->isActive())

	if (!overlayFont.loadFromFile("../font/Inconsolata.otf")) {
		throw std::runtime_error("Could not load font!");
	}

	circle = Mesh::GenerateCircle(1, 16);

	screenWidth = pWindow->getSize().x;
	screenHeight = pWindow->getSize().y;

	textDrawer.SetScreenResolution(gl::Vec2i(screenWidth, screenHeight));

	framebuffers.push_back(std::move(Framebuffer(screenWidth, screenHeight)));
	framebuffers.back().Bind(gl::Framebuffer::Target::Draw);

	finalFramebufferCopy = LoadShaderProgramFromFiles("finalFramebufferCopy_v.glsl", "finalFramebufferCopy_f.glsl");
	finalFramebufferCopy.Use();

	framebufferCopy_ScreenWidth = gl::Uniform<GLint>(finalFramebufferCopy, "screenWidth");
	framebufferCopy_ScreenHeight = gl::Uniform<GLint>(finalFramebufferCopy, "screenHeight");
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	gl::UniformSampler(finalFramebufferCopy, "colorTex").Set(0);
	gl::UniformSampler(finalFramebufferCopy, "depthTex").Set(1);

	//////////
	//Init objects

	//terrain.LoadFromHeightMap(DemoCore::imgFolderPath + "heightMap.png", terrainSize, 0.06);
	terrain.LoadFromHeightMap(DemoCore::imgFolderPath + "heightMap.png", terrainSize, 0.2);

	//NOTE: rotation angle does intentionally differ from exactly pi/2.
	//Reason: oglplus's matrix inversion function doesn't invert correctly for some transforms.
	terrain.SetTransform(gl::ModelMatrixf::Translation(-terrainSize*0.5, -waterLevel, terrainSize*0.5)*gl::ModelMatrixf::RotationA(gl::Vec3f(1, 0, 0), gl::Radians<float>(-gl::math::Pi() / 2.001)));

	skybox.LoadTextureFromFiles(
		DemoCore::imgFolderPath + "sb4-x.bmp",
		DemoCore::imgFolderPath + "sb1+x.bmp",
		DemoCore::imgFolderPath + "sb6-y.bmp",
		DemoCore::imgFolderPath + "sb3+y.bmp",
		DemoCore::imgFolderPath + "sb2-z.bmp",
		DemoCore::imgFolderPath + "sb5+z.bmp");

	cam.SetFovY(gl::Degrees(70));

	sun.SetColor(gl::Vec3f(1, 1, 1));

	/*
	{
		Mesh mesh;
		mesh.LoadFromFile(DemoCore::modelsFolderPath + "CerberusCycles.obj");

		GraphicalObject object;
		object.SetMesh(std::move(mesh));

		const float headSize = 2.00;

		//object.SetTransform(gl::ModelMatrixf::Translation(0, -0.5, -25) * gl::ModelMatrixf::RotationZ(gl::Radians(gl::math::Pi()*0.2)) * gl::ModelMatrixf::RotationY(gl::Radians(gl::math::Pi()*0.45)) * gl::ModelMatrixf::Scale(headSize, headSize, headSize));
		object.SetTransform(gl::ModelMatrixf::Translation(0, 3, -25) * gl::ModelMatrixf::Scale(1, 1, 1));

		graphicalObjects.push_back(std::move(object));
	}
	*/

	/////////////////
	//Init more variables

	currentSpeedMode = SpeedMode::NORMAL;
	wireframeModeEnabled = false;

	isTrackingMouse = false;

	forwardMovement = 0;
	rightMovement = 0;

	cam.SetScreenWidth(screenWidth);
	cam.SetScreenHeight(screenHeight);
	cam.SetPosition(gl::Vec3f(0, 5, 10));

}

int DemoCore::Start()
{
	running = true;

	pWindow->setKeyRepeatEnabled(false);

	glContext.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glContext.ClearDepth(1.0f);
	glContext.Enable(gl::enums::Capability::DepthTest);
	glContext.Enable(gl::enums::Capability::CullFace);

	double lastFPSUpdateSec = 0;
	double FPSUpdateDelaySec = 0.1;

	int framesSinceLastFPSUpdate = 0;
	int recentMinFPS = 5000;

	double lastMinResetSec = 0;
	double longestMinKeepSec = 7;

	double lastUpdateSec = 0;

	while (running) {

		elapsedSec = clock.getElapsedTime().asSeconds();
		double deltaSec = elapsedSec - lastUpdateSec;
		lastUpdateSec = elapsedSec;

		sf::Event event;
		while (pWindow->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running = false;
			}

			if (event.type == sf::Event::KeyPressed) {
				KeyPressed(event.key);
			}
			else if (event.type == sf::Event::KeyReleased) {
				KeyReleased(event.key);
			}
			if (event.type == sf::Event::MouseMoved) {
				MouseMoved();
			}
			if (event.type == sf::Event::MouseWheelMoved) {
				MouseWheelMoved(event.mouseWheel);
			}
			if (event.type == sf::Event::Resized) {
				Resize(event.size.width, event.size.height);
			}
		}

		float currSpeed = GetCurrentSpeed();

		cam.MoveForward(forwardMovement * currSpeed * deltaSec);
		cam.MoveRight(rightMovement * currSpeed * deltaSec);

		Update(deltaSec);
		Draw();

		if (elapsedSec - lastMinResetSec > longestMinKeepSec) {
			recentMinFPS = 5000;
			lastMinResetSec = elapsedSec;
		}
		else {
			recentMinFPS = std::min(int(currFPS), recentMinFPS);
		}

		if (elapsedSec - lastFPSUpdateSec > FPSUpdateDelaySec) {
			int updatedFPS = std::round(framesSinceLastFPSUpdate / (elapsedSec - lastFPSUpdateSec));
			pWindow->setTitle(sf::String("Glitter-Island <| FPS: ") + std::to_string(updatedFPS) + " current; " + std::to_string(recentMinFPS) + " recent min |>");
			lastFPSUpdateSec = elapsedSec;
			framesSinceLastFPSUpdate = 0;
		}
		else {
			framesSinceLastFPSUpdate++;
		}

		pWindow->display();
	}

	return EXIT_SUCCESS;
}

void DemoCore::PushFramebuffer()
{
	framebuffers.push_back(std::move(Framebuffer(screenWidth, screenHeight)));
	framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
}

void DemoCore::PopFramebuffer()
{
	if (framebuffers.size() > 1) {
		framebuffers.pop_back();
		framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
	}
}

void DemoCore::CopyFramebufferContents(const Framebuffer& source)
{
	source.Bind(gl::Framebuffer::Target::Read);
	glContext.BlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, gl::Bitfield<gl::enums::BufferSelectBit>(gl::enums::BufferSelectBit::DepthBuffer) |= gl::enums::BufferSelectBit::ColorBuffer, gl::enums::BlitFilter::Nearest);
}

Framebuffer& DemoCore::GetCurrentFramebuffer()
{
	return framebuffers.back();
}

gl::Context& DemoCore::GetGLContext()
{
	return glContext;
}

sf::Time DemoCore::GetElapsedTime()
{
	return sf::seconds(elapsedSec);
}

bool DemoCore::GetWireframeModeEnabled() const
{
	return wireframeModeEnabled;
}

DirectionalLight& DemoCore::GetSun()
{
	return sun;
}

Camera& DemoCore::GetCamera()
{
	return cam;
}

const DirectionalLight& DemoCore::GetSun() const
{
	return sun;
}

const Camera& DemoCore::GetCamera() const
{
	return cam;
}


////////////////////////////////////////////
//
// Private functions
//
////////////////////////////////////////////

float DemoCore::GetCurrentSpeed() const
{
	float result;

	switch (currentSpeedMode) {
	case SpeedMode::NORMAL:
		result = camSpeed;
		break;
	case SpeedMode::FAST:
		result = camSpeed * fastSpeedMultiplyer;
		break;

	case SpeedMode::ULTRA:
		result = camSpeed * ultraSpeedMultiplyer;
		break;
	}

	return result;
}

void DemoCore::ClearFramebufferStack()
{
	while (framebuffers.size() > 1) {
		framebuffers.pop_back();
	}

	framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
}

void DemoCore::Resize(const int width, const int height)
{
	screenWidth = width;
	screenHeight = height;
	GetGLContext().Viewport(0, 0, screenWidth, screenHeight);

	cam.SetScreenWidth(screenWidth);
	cam.SetScreenHeight(screenHeight);

	textDrawer.SetScreenResolution(gl::Vec2i(screenWidth, screenHeight));

	finalFramebufferCopy.Use();
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	for (auto& current : framebuffers) {
		current.SetResolution(width, height);
	}
}

void DemoCore::MouseMoved()
{
	//moved mouse tracikg to update
}

void DemoCore::MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent)
{
	if (isInEditorMode) {
		brushRadius += -wheelEvent.delta*0.5;
		brushRadius = std::max(brushRadius, 0.5f);
	}
}

void DemoCore::KeyPressed(sf::Event::KeyEvent key)
{
	switch (key.code) {
	case sf::Keyboard::Escape:
		running = false;
		break;
	case sf::Keyboard::Space:
		isTrackingMouse = !isTrackingMouse;
		pWindow->setMouseCursorVisible(!isTrackingMouse);
		if (isTrackingMouse) {
			sf::Mouse::setPosition(sf::Vector2i(screenWidth / 2, screenHeight / 2), *pWindow);
		}
		break;

	case sf::Keyboard::H:
		wireframeModeEnabled = !wireframeModeEnabled;
		break;

	case sf::Keyboard::E:
		isInEditorMode = !isInEditorMode;
		break;

	case sf::Keyboard::LShift:
		currentSpeedMode = SpeedMode::FAST;
		break;
	case sf::Keyboard::LControl:
		currentSpeedMode = SpeedMode::ULTRA;
		break;
	case sf::Keyboard::W:
		forwardMovement = 1;
		break;
	case sf::Keyboard::S:
		forwardMovement = -1;
		break;
	case sf::Keyboard::A:
		rightMovement = -1;
		break;
	case sf::Keyboard::D:
		rightMovement = 1;
		break;

    default:
        break;
	}

	static int saveCount = 0;

	if (isInEditorMode) {
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
			pWindow->setTitle("Saving...");
			SaveAll();
			std::cout << saveCount++ << " Saved!" << std::endl;
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
}

void DemoCore::KeyReleased(sf::Event::KeyEvent key)
{
	switch (key.code) {
	case sf::Keyboard::LShift:
		if (currentSpeedMode == SpeedMode::FAST) {currentSpeedMode = SpeedMode::NORMAL;}
		break;
	case sf::Keyboard::LControl:
		if (currentSpeedMode == SpeedMode::ULTRA) {currentSpeedMode = SpeedMode::NORMAL;}
		break;
	case sf::Keyboard::W:
		if (forwardMovement > 0) {
			forwardMovement = 0;
		}
		break;
	case sf::Keyboard::S:
		if (forwardMovement < 0) {
			forwardMovement = 0;
		}
		break;
	case sf::Keyboard::A:
		if (rightMovement < 0) {
			rightMovement = 0;
		}
		break;
	case sf::Keyboard::D:
		if (rightMovement > 0) {
			rightMovement = 0;
		}
		break;

    default:
        break;
	}
}

void DemoCore::DisplayModelSelection()
{

}

void DemoCore::UpdatePointPosAtCursor()
{
	sf::Vector2i cursorPos = sf::Mouse::getPosition(*pWindow);

	//invert y to suit opengl coordinates
	cursorPos.y = screenHeight-cursorPos.y;

	GLfloat depthAtPixel;
	glContext.ReadPixels(cursorPos.x, cursorPos.y, 1, 1, gl::enums::PixelDataFormat::DepthComponent, gl::PixelDataType::Float, &depthAtPixel);

	pointPosAtCursor = gl::Inverse(cam.GetViewProjectionTransform()) * gl::Vec4f(
		((float(cursorPos.x)/screenWidth)*2-1),
		((float(cursorPos.y)/screenHeight)*2-1),
		(depthAtPixel)*2-1,
		1);

	pointPosAtCursor = pointPosAtCursor / pointPosAtCursor.w();
}

void DemoCore::UpdateModelFileList()
{
	// Find all model files in models directory
	auto tmpList = Util::GetFileNamesInDirectory("../models");

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

void DemoCore::Update(float deltaSec)
{
	sf::Vector2i sfCursorPos = sf::Mouse::getPosition(*pWindow);

	if (isTrackingMouse) {
		const sf::Vector2i windowCenter = sf::Vector2i(screenWidth / 2, screenHeight / 2);

		cam.RotateHorizontally(gl::Radians(-1.f*(sfCursorPos.x - windowCenter.x) * mouseSensitivity));
		cam.RotateVertically(gl::Radians(-1.f*(sfCursorPos.y - windowCenter.y) * mouseSensitivity));

		if (sfCursorPos != windowCenter) {
			sf::Mouse::setPosition(windowCenter, *pWindow);
		}
	}

	currFPS = 1/deltaSec;

	//sun.SetDirectionTowardsSource(gl::Vec3f(std::cos(GetElapsedTime().asSeconds()), 1.5, std::sin(GetElapsedTime().asSeconds())));
	sun.SetDirectionTowardsSource(gl::Vec3f(1, 1, -1));

    if (isInEditorMode) {
    	UpdatePointPosAtCursor();

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

			switch (GetToolType(selectedTool)) {
			case EditorToolType::PAINT: {
				gl::Vec2i cursorPosOnMaterialMap = terrain.GetMaterialMapPos(pointPosAtCursor);
				sf::Image& materialMap = terrain.GetMaterialMap();
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

				terrain.DownloadMaterialMapToGPU();
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
}

void DemoCore::Draw()
{
	ClearFramebufferStack();

	if (wireframeModeEnabled) {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Line);
	}
	else {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	}

	glContext.Enable(gl::Capability::DepthTest);
	GetCurrentFramebuffer().Bind(gl::Framebuffer::Target::Draw);

	DrawScene();

	//draw current framebuffer to screen
	GetCurrentFramebuffer().SetVertexPosName("vertexPos");
	GetCurrentFramebuffer().SetShaderProgram(&finalFramebufferCopy);

	glContext.Enable(gl::Capability::DepthTest);
	defaultFBO.Bind(gl::Framebuffer::Target::Draw);
	glContext.Clear().ColorBuffer().DepthBuffer();

	GetCurrentFramebuffer().Draw(*this);

	DrawOverlay();
}

void DemoCore::DrawScene()
{
	glContext.Clear().ColorBuffer().DepthBuffer();
	glContext.DepthFunc(gl::enums::CompareFunction::LEqual);

	glContext.Enable(gl::Capability::CullFace);
	DrawObjects();

	skybox.Draw(*this);

	glContext.Disable(gl::Capability::CullFace);
	water.Draw(*this);

	//(TODO) WARNING: Drawing Skybox twice! It's only purpose is to make water fade out.
	//Please note that drawing skybox ONLY after water is not a good souliton, beacuse without a skybox behind water, it will refract black background making distant water black.
	skybox.Draw(*this);
}

void DemoCore::DrawObjects()
{
	gl::Mat4f projection = cam.GetProjectionTransform();
	gl::Mat4f view = cam.GetViewTransform();
	gl::Mat4f viewProjection = projection*view;

	terrain.Draw(*this);

	for (auto& current : graphicalObjects) {
		current.Draw(*this);
	}

	if (isInEditorMode) {
		DrawEditorMode();
	}
}

void DemoCore::DrawEditorMode()
{
	EditorToolType selectedToolType = GetToolType(selectedTool);
	if (selectedToolType == EditorToolType::PAINT) {
		float meshScale = terrain.GetMaterialMapPixelSizeInWorldScale() * brushRadius;
		using ModelMatf = gl::ModelMatrixf;
		gl::Mat4f MVP =
			cam.GetViewProjectionTransform() *
			ModelMatf::Translation(pointPosAtCursor.xyz()) *
			ModelMatf::Scale(meshScale, meshScale, meshScale) *
			//ModelMatf::RotationY(gl::Radians<float>(gl::math::Pi()*0.25)) *
			ModelMatf::RotationX(gl::Radians<float>(gl::math::Pi()*0.5));

		//glContext.Disable(gl::Capability::CullFace);
		glContext.Disable(gl::Capability::DepthTest);
		glContext.LineWidth(2);
		simpleColoredDrawer.Draw(glContext, circle, MVP, gl::Vec4f(1, 0.1, 0.5, 1));
		glContext.LineWidth(1);
		glContext.Enable(gl::Capability::DepthTest);
		//glContext.Enable(gl::Capability::CullFace);
	}
}

void DemoCore::DrawOverlay()
{
	if (isInEditorMode) {
		//glContext.Clear().DepthBuffer();
		glContext.Disable(gl::Capability::DepthTest);
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
		glContext.Enable(gl::Capability::Blend);
		glContext.BlendFunc(gl::BlendFunction::SrcAlpha, gl::BlendFunction::OneMinusSrcAlpha);

		sf::Text text("-Editor mode-", overlayFont);
		text.setPosition(sf::Vector2f(30, 25));
		text.setCharacterSize(18);
		text.setColor(sf::Color(208, 59, 237, ~sf::Uint8(0)));
		text.setStyle(sf::Text::Bold);
		textDrawer.DrawBackground(glContext, text, sf::Color(50, 50, 50, 150), 5);
		textDrawer.Draw(glContext, text);

		
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
		textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 150), 5);
		textDrawer.DrawAsList(glContext, text, GetToolInfo(selectedTool).id+1, sf::Color::Cyan);

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
			textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 150), 5);
			textDrawer.DrawAsList(glContext, text, howeredModelID-listFirstElementID, sf::Color::Cyan);
		}


		glContext.Disable(gl::Capability::Blend);
	}
}

void DemoCore::SaveAll()
{
	terrain.SaveMaterialMap();
}
