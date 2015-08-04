#include "BaseDemoContext.hpp"

#include "DemoCore.hpp"

BaseDemoContext::BaseDemoContext(ContextManager* pContextManager, DemoCore* pDemoCore) : 
GUIContext(pContextManager, pDemoCore),
camSpeed(3), fastSpeedMultiplyer(8.5), ultraSpeedMultiplyer(30),
terrainSize(500), waterLevel(49), water(terrainSize * 7),
editorContext(pContextManager, pDemoCore)
{
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

	currentSpeedMode = SpeedMode::NORMAL;
	wireframeModeEnabled = false;

	isTrackingMouse = false;

	forwardMovement = 0;
	rightMovement = 0;
}

BaseDemoContext::~BaseDemoContext()
{}

void BaseDemoContext::HandleWindowEvent(const sf::Event& event)
{
	const int screenHeight = pDemoCore->GetCamera().GetScreenHeight();
	const int screenWidth = pDemoCore->GetCamera().GetScreenWidth();

	if (event.type == sf::Event::KeyPressed) {
		KeyPressed(event.key);
	}
	else if (event.type == sf::Event::KeyReleased) {
		KeyReleased(event.key);
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Right) {
			isTrackingMouse = true;
			pDemoCore->GetWindow()->setMouseCursorVisible(!isTrackingMouse);
			sf::Mouse::setPosition(sf::Vector2i(screenWidth / 2, screenHeight / 2), *pDemoCore->GetWindow());
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Button::Right) {
			isTrackingMouse = false;
			pDemoCore->GetWindow()->setMouseCursorVisible(!isTrackingMouse);
		}
	}
}

void BaseDemoContext::EnteringContext() 
{
}

void BaseDemoContext::LeavingContext()
{
}

void BaseDemoContext::Update(float deltaSec) 
{
	float currSpeed = GetCurrentSpeed();

	Camera& cam = pDemoCore->GetCamera();

	cam.MoveForward(forwardMovement * currSpeed * deltaSec);
	cam.MoveRight(rightMovement * currSpeed * deltaSec);

	sf::Vector2i sfCursorPos = sf::Mouse::getPosition(*pDemoCore->GetWindow());

	if (isTrackingMouse) {
		const sf::Vector2i windowCenter = sf::Vector2i(cam.GetScreenWidth() / 2, cam.GetScreenHeight() / 2);

		cam.RotateHorizontally(gl::Radians(-1.f*(sfCursorPos.x - windowCenter.x) * pDemoCore->GetMouseSensitivity()));
		cam.RotateVertically(gl::Radians(-1.f*(sfCursorPos.y - windowCenter.y) * pDemoCore->GetMouseSensitivity()));

		if (sfCursorPos != windowCenter) {
			// Might be too agressive, but can't think of a better solution for now.
			sf::Mouse::setPosition(windowCenter, *pDemoCore->GetWindow());
		}
	}

	//sun.SetDirectionTowardsSource(gl::Vec3f(std::cos(GetElapsedTime().asSeconds()), 1.5, std::sin(GetElapsedTime().asSeconds())));
	sun.SetDirectionTowardsSource(gl::Vec3f(1, 1, -1));
}

void BaseDemoContext::Draw() 
{
	gl::Context& glContext = pDemoCore->GetGLContext();

	if (wireframeModeEnabled) {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Line);
	}
	else {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	}

	glContext.Enable(gl::Capability::DepthTest);

	glContext.Clear().ColorBuffer().DepthBuffer();
	glContext.DepthFunc(gl::enums::CompareFunction::LEqual);

	glContext.Enable(gl::Capability::CullFace);
	DrawObjects();

	skybox.Draw(*pDemoCore);

	water.Draw(*pDemoCore);

	//(TODO) WARNING: Drawing Skybox twice! It's only purpose is to make water fade out.
	//Please note that drawing skybox ONLY after water is not a good solution, because without a skybox behind water, it will refract black background making distant water black.
	skybox.Draw(*pDemoCore);
}

void BaseDemoContext::DrawOverlayElements()
{}

void BaseDemoContext::AddGraphicalObject(GraphicalObject&& newObject)
{
	graphicalObjects.push_back(std::move(newObject));
}

float BaseDemoContext::GetCurrentSpeed() const
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

bool BaseDemoContext::GetWireframeModeEnabled() const
{
	return wireframeModeEnabled;
}

DirectionalLight& BaseDemoContext::GetSun()
{
	return sun;
}

Terrain& BaseDemoContext::GetTerrain()
{
	return terrain;
}

void BaseDemoContext::KeyPressed(sf::Event::KeyEvent key)
{
	const int screenHeight = pDemoCore->GetCamera().GetScreenHeight();
	const int screenWidth = pDemoCore->GetCamera().GetScreenWidth();

	switch (key.code) {
	case sf::Keyboard::Escape:
		//running = false;
		pDemoCore->Stop();
		break;
	case sf::Keyboard::Z:
		isTrackingMouse = !isTrackingMouse;
		pDemoCore->GetWindow()->setMouseCursorVisible(!isTrackingMouse);
		if (isTrackingMouse) {
			sf::Mouse::setPosition(sf::Vector2i(screenWidth / 2, screenHeight / 2), *pDemoCore->GetWindow());
		}
		break;

	case sf::Keyboard::H:
		wireframeModeEnabled = !wireframeModeEnabled;
		break;

	case sf::Keyboard::E: {
		pContextManager->PushContext(&editorContext);
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

void BaseDemoContext::KeyReleased(sf::Event::KeyEvent key)
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

void BaseDemoContext::DrawObjects()
{
	Camera& cam = pDemoCore->GetCamera();

	gl::Mat4f projection = cam.GetProjectionTransform();
	gl::Mat4f view = cam.GetViewTransform();
	gl::Mat4f viewProjection = projection*view;

	terrain.Draw(*pDemoCore);

	for (auto& current : graphicalObjects) {
		current.Draw(*pDemoCore);
		//pDemoCore->simpleColoredDrawer.Draw(pDemoCore->GetGLContext(), current.GetMesh(), cam.GetViewProjectionTransform() * current.GetTransform(), gl::Vec4f(1, 1, 0, 0.7));
	}
}
