#include "DemoCore.hpp"

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "FileLoad.hpp"

const std::string DemoCore::shadersFolderPath = "../shaders/";
const std::string DemoCore::imgFolderPath = "../img/";
const std::string DemoCore::modelsFolderPath = "../models/";

DemoCore::EditToolType DemoCore::GetToolType(DemoCore::EditTool tool)
{
	switch (tool) {
	case EditTool::PAINT_FLAT_SAND:
	case EditTool::PAINT_SAND_TEXTURE:
	case EditTool::PAINT_GRASS_TEXTURE:
		return EditToolType::PAINT;
		break;

	case EditTool::SPAWN_GRASS_BUNCH:
	case EditTool::SPAWN_ROCK_BUNCH:
		return EditToolType::SPAWN;
		break;

	case EditTool::PLACE_MODEL:
		return EditToolType::PLACE;
		break;

	case EditTool::NO_TOOL:
		return EditToolType::NO_TOOL;
		break;

	default:
		assert(false);
		break;
	}
}

gl::Program DemoCore::LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name)
{
	gl::Program result;

	gl::VertexShader vs;
	vs.Source(LoadFileAsString(shadersFolderPath + vs_name));
	vs.Compile();

	gl::FragmentShader fs;
	fs.Source(LoadFileAsString(shadersFolderPath + fs_name));
	fs.Compile();

	result.AttachShader(vs);
	result.AttachShader(fs);
	result.Link();

	return result;
}

DemoCore::DemoCore(sf::Window* pWindow) :
running(false),
isInEditMode(false), selectedTool(EditTool::NO_TOOL),
pWindow(pWindow),
mouseSensitivity(0.005), camSpeed(2.5), fastSpeedMultiplyer(10),
terrainSize(500), waterLevel(49), water(terrainSize * 7)
{
	//assert(pWindow->isActive())

	if (!overlayFont.loadFromFile("../font/Inconsolata.otf")) {
		throw std::runtime_error("Could not load font!");
	}

	circle = Mesh::GenerateCircle(1, 10);

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

	isInFastMode = false;
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
			if (event.type == sf::Event::Resized) {
				Resize(event.size.width, event.size.height);
			}
		}

		float currSpeed = camSpeed * (isInFastMode ? fastSpeedMultiplyer : 1);

		cam.MoveForward(forwardMovement * currSpeed * deltaSec);
		cam.MoveRight(rightMovement * currSpeed * deltaSec);

		Update();
		Draw();

		if (elapsedSec - lastMinResetSec > longestMinKeepSec) {
			recentMinFPS = 5000;
			lastMinResetSec = elapsedSec;
		}
		else {
			recentMinFPS = 1/deltaSec < recentMinFPS ? 1/deltaSec : recentMinFPS;
		}

		if (elapsedSec - lastFPSUpdateSec > FPSUpdateDelaySec) {
			int currFPS = std::round(framesSinceLastFPSUpdate / (elapsedSec - lastFPSUpdateSec));
			pWindow->setTitle(sf::String("Glitter-Island <| FPS: ") + std::to_string(currFPS) + " current; " + std::to_string(recentMinFPS) + " recent min |>");
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
	//return clock.getElapsedTime();
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
	//GetCurrentFramebuffer().SetResolution(width, height);
}

void DemoCore::MouseMoved()
{
	if (isTrackingMouse) {
		const sf::Vector2i windowCenter = sf::Vector2i(screenWidth / 2, screenHeight / 2);
		const sf::Vector2i currPos = sf::Mouse::getPosition(*pWindow);

		cam.RotateHorizontally(gl::Radians(-1.f*(currPos.x - windowCenter.x) * mouseSensitivity));
		cam.RotateVertically(gl::Radians(-1.f*(currPos.y - windowCenter.y) * mouseSensitivity));

		if (currPos != windowCenter) {
			sf::Mouse::setPosition(windowCenter, *pWindow);
		}
	}
}

void DemoCore::KeyPressed(sf::Event::KeyEvent key)
{
	switch (key.code){
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
		isInEditMode = !isInEditMode;
		break;

	case sf::Keyboard::LShift:
		isInFastMode = true;
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

	if (isInEditMode) {
		switch (key.code){
		case sf::Keyboard::Num0:
			selectedTool = EditTool::NO_TOOL;
			break;
		case sf::Keyboard::Num1:
			selectedTool = EditTool::PAINT_FLAT_SAND;
			break;
		case sf::Keyboard::Num2:
			selectedTool = EditTool::PAINT_SAND_TEXTURE;
			break;
		case sf::Keyboard::Num3:
			selectedTool = EditTool::PAINT_GRASS_TEXTURE;
			break;
		case sf::Keyboard::Num4:
			selectedTool = EditTool::SPAWN_GRASS_BUNCH;
			break;
		case sf::Keyboard::Num5:
			selectedTool = EditTool::SPAWN_ROCK_BUNCH;
			break;
		case sf::Keyboard::Num6:
			selectedTool = EditTool::PLACE_MODEL;
			break;
		default:
			break;
		}
	}
}

void DemoCore::KeyReleased(sf::Event::KeyEvent key)
{
	switch (key.code){
	case sf::Keyboard::LShift:
		isInFastMode = false;
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

void DemoCore::Update()
{
	//sun.SetDirectionTowardsSource(gl::Vec3f(std::cos(GetElapsedTime().asSeconds()), 1.5, std::sin(GetElapsedTime().asSeconds())));
	sun.SetDirectionTowardsSource(gl::Vec3f(1, 1, -1));

    if (isInEditMode) {
    	UpdatePointPosAtCursor();

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

			// 1, Calculate cursors pos on terrain material map (from cursors world pos)
			// 2, Set pixels on terrain material map according to the selected tool
			// 3, (Download material map to GPU to make changes visible)

			switch (GetToolType(selectedTool)) {
			case EditToolType::PAINT: {
				gl::Vec2i cursorPosOnMaterialMap = terrain.GetMaterialMapPos(pointPosAtCursor);
				sf::Image& materialMap = terrain.GetMaterialMap();
				sf::Color selectedMaterialColor;
				if (selectedTool == EditTool::PAINT_FLAT_SAND) {
					selectedMaterialColor = sf::Color(0, 0, 0, 255);
				}
				else if(selectedTool == EditTool::PAINT_SAND_TEXTURE) {
					selectedMaterialColor = sf::Color(0, 0, 255, 255);
				}
				else if(selectedTool == EditTool::PAINT_GRASS_TEXTURE) {
					selectedMaterialColor = sf::Color(0, 255, 0, 255);
				}

				materialMap.setPixel(cursorPosOnMaterialMap.x(),
										cursorPosOnMaterialMap.y(),
										selectedMaterialColor);

				terrain.UpdateMaterialMap();
				break;
			}
			case EditToolType::SPAWN: {
				break;
			}
			case EditToolType::PLACE: {
				break;
			}
			case EditToolType::NO_TOOL: {
				break;
			}
			default:
				assert(false);
			}

			/*
			switch (selectedTool) {
			//TODO delete this test code
			default:
				std::cout << "Hmm..." << std::endl;
			break;

			case EditTool::PAINT_FLAT_SAND:
				break;

			case EditTool::PAINT_SAND_TEXTURE:
				break;

			case EditTool::PAINT_GRASS_TEXTURE:
				break;

			case EditTool::SPAWN_GRASS_BUNCH:
				break;

			case EditTool::SPAWN_ROCK_BUNCH:
				break;

			case EditTool::PLACE_MODEL:
				break;

			case EditTool::NO_TOOL:
				break;
			}*/
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

	terrain.SetLightDir(gl::Normalized(sun.GetDirectionTowardsSource()));
	//terrain.SetTransform(gl::ModelMatrixf::Translation(-terrainSize*0.5, -waterLevel, terrainSize*0.5)*gl::ModelMatrixf::RotationA(gl::Vec3f(1, 0, 0), gl::Radians<float>(-gl::math::Pi() / 2)));
	terrain.Draw(*this);

	for (auto& current : graphicalObjects) {
		current.Draw(*this);
	}

	if (isInEditMode) {
		DrawEditMode();
	}
}

void DemoCore::DrawEditMode()
{
	EditToolType selectedToolType = GetToolType(selectedTool);
	if (selectedToolType == EditToolType::PAINT) {
		gl::Mat4f MVP = cam.GetViewProjectionTransform() * gl::ModelMatrixf::Translation(pointPosAtCursor.xyz()) * gl::ModelMatrixf::RotationX(gl::Radians<float>(gl::math::Pi()*0.5));

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
	if (isInEditMode) {
		//glContext.Clear().DepthBuffer();
		glContext.Disable(gl::Capability::DepthTest);
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
		glContext.Enable(gl::Capability::Blend);
		glContext.BlendFunc(gl::BlendFunction::SrcAlpha, gl::BlendFunction::OneMinusSrcAlpha);

		sf::Text text("-Edit mode-", overlayFont);
		text.setPosition(sf::Vector2f(30, 25));
		text.setCharacterSize(18);
		text.setColor(sf::Color(208, 59, 237, ~sf::Uint8(0)));
		text.setStyle(sf::Text::Bold);
		textDrawer.DrawBackground(glContext, text, sf::Color(50, 50, 50, 150), 5);
		textDrawer.Draw(glContext, text);

		text.setString(
			"E -- toggle edit mode\n"
			"0 -- select no tool\n"
			"1 -- paint flat sand\n"
			"2 -- paint sand texture\n"
			"3 -- paint grass texture\n"
			"4 -- spawn grass bunch\n"
			"5 -- spawn rock bunch\n"
			"6 -- place model"
			);
		text.setPosition(sf::Vector2f(2, 100));
		text.setCharacterSize(16);
		text.setStyle(sf::Text::Regular);
		//text.setColor(sf::Color(205, 34, 240, ~sf::Uint8(0)));
		text.setColor(sf::Color::Yellow);
		textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 150), 5);
		textDrawer.Draw(glContext, text);

		glContext.Disable(gl::Capability::Blend);
	}
}
