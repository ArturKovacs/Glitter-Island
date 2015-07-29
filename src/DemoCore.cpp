#include "DemoCore.hpp"

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "Utility.hpp"

const std::string DemoCore::shadersFolderPath = "../shaders/";
const std::string DemoCore::imgFolderPath = "../img/";
const std::string DemoCore::modelsFolderPath = "../models/";

gl::Program DemoCore::LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name)
{
	gl::Program result;

	gl::VertexShader vs;
	vs.Source(Util::LoadFileAsString(shadersFolderPath + vs_name));
	try {
		vs.Compile();
	}
	catch (gl::Error& err) {
		throw std::runtime_error(std::string(err.what()) + "\n\nIn file: " + vs_name + "\n\nLog:\n" + err.Log());
	}

	gl::FragmentShader fs;
	fs.Source(Util::LoadFileAsString(shadersFolderPath + fs_name));
	try {
		fs.Compile();
	}
	catch (gl::Error& err) {
		throw std::runtime_error(std::string(err.what()) + "\n\nIn file: " + fs_name + "\n\nLog:\n" + err.Log());
	}

	result.AttachShader(vs);
	result.AttachShader(fs);
	result.Link();

	return result;
}

DemoCore::DemoCore(sf::Window* pWindow) :
GUIContext(this),
running(false),
//isInEditorMode(false), selectedTool(EditorTool::NO_TOOL), brushRadius(1), showModelSelection(false), howeredModelID(0), selectedModelID(0),
editorContext(this),
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

	//gl::UniformSampler(finalFramebufferCopy, "colorTex").Set(0);
	//gl::UniformSampler(finalFramebufferCopy, "depthTex").Set(1);

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

	activeGUIStack.push_back(this);
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
				//KeyPressed(event.key);
				ContextManagerKeyPressed(event.key);
			}
			else if (event.type == sf::Event::KeyReleased) {
				//KeyReleased(event.key);
				ContextManagerKeyReleased(event.key);
			}
			if (event.type == sf::Event::MouseMoved) {
//				MouseMoved();
			}
			if (event.type == sf::Event::MouseWheelMoved) {
				//MouseWheelMoved(event.mouseWheel);
				ContextManagerMouseWheelMoved(event.mouseWheel);
			}
			if (event.type == sf::Event::Resized) {
				Resize(event.size.width, event.size.height);
			}
		}

		float currSpeed = GetCurrentSpeed();

		cam.MoveForward(forwardMovement * currSpeed * deltaSec);
		cam.MoveRight(rightMovement * currSpeed * deltaSec);

		ContextManagerUpdate(deltaSec);
		ContextManagerDraw();

		//Update(deltaSec);
		//Draw();

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

Terrain& DemoCore::GetTerrain()
{
	return terrain;
}

const DirectionalLight& DemoCore::GetSun() const
{
	return sun;
}

const Camera& DemoCore::GetCamera() const
{
	return cam;
}

sf::Window* DemoCore::GetWindow()
{
	return pWindow;
}

void DemoCore::SaveAll()
{
	static int saveCount = 0;
	pWindow->setTitle("Saving...");

	terrain.SaveMaterialMap();

	std::cout << saveCount++ << " Saved!" << std::endl;
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

void DemoCore::ContextManagerUpdate(float deltaSec)
{
	for (auto current : activeGUIStack) {
		current->Update(deltaSec);
	}
}

void DemoCore::ContextManagerDraw()
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

	//DrawScene();
	for (auto current : activeGUIStack) {
		current->Draw();
	}
	//activeGUIStack.front()->Draw();

	simpleColoredDrawer.Draw(glContext, circle, cam.GetViewProjectionTransform(), gl::Vec4f(1, 1, 0, 1));

	//draw current framebuffer to screen
	GetCurrentFramebuffer().SetVertexPosName("vertexPos");
	GetCurrentFramebuffer().SetColorTexName("colorTex");
	GetCurrentFramebuffer().SetDepthTexName("depthTex");
	GetCurrentFramebuffer().SetShaderProgram(&finalFramebufferCopy);

	defaultFBO.Bind(gl::Framebuffer::Target::Draw);
	glContext.Clear().ColorBuffer().DepthBuffer();

	GetCurrentFramebuffer().Draw(*this);

	for (auto current : activeGUIStack) {
		current->DrawOverlayElements();
	}
}

void DemoCore::ContextManagerMouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent)
{
	if (activeGUIStack.back()->GetRequireFocus()) {
		activeGUIStack.back()->MouseWheelMoved(wheelEvent);
	}
	else {
		for (auto current : activeGUIStack) {
			current->MouseWheelMoved(wheelEvent);
		}
	}
}

void DemoCore::ContextManagerKeyPressed(sf::Event::KeyEvent key)
{
	if (activeGUIStack.back()->GetRequireFocus()) {
		activeGUIStack.back()->KeyPressed(key);
	}
	else {
		//Do not use iterators here, called function might change activeGUI stack (is this an error?)
		for (int i = 0; i < activeGUIStack.size(); i++) {
			activeGUIStack[i]->KeyPressed(key);
		}
	}
}

void DemoCore::ContextManagerKeyReleased(sf::Event::KeyEvent key)
{
	if (activeGUIStack.back()->GetRequireFocus()) {
		activeGUIStack.back()->KeyReleased(key);
	}
	else {
		for (int i = 0; i < activeGUIStack.size(); i++) {
			activeGUIStack[i]->KeyReleased(key);
		}
	}
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

void DemoCore::EnteringContext()
{
}

void DemoCore::LeavingContext()
{
}


void DemoCore::MouseWheelMoved(sf::Event::MouseWheelEvent wheelEvent)
{
	//if (isInEditorMode) {

	//}
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

	case sf::Keyboard::E: {
		//TODO enter editor mode
		//isInEditorMode = !isInEditorMode;
		//Push editor context if its not there, remove it, if its there
		auto editorContextIterator = std::find(activeGUIStack.begin(), activeGUIStack.end(), &editorContext);
		if (editorContextIterator == activeGUIStack.end()) {
			activeGUIStack.push_back(&editorContext);
			editorContext.EnteringContext();
		}
		else {
			editorContext.LeavingContext();
			activeGUIStack.erase(editorContextIterator);
		}
		break;
	}

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
}

void DemoCore::Draw()
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

void DemoCore::DrawOverlayElements()
{
}

void DemoCore::DrawScene()
{
	
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
}
