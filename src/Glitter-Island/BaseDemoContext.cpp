#include "BaseDemoContext.hpp"

#include "DemoCore.hpp"

BaseDemoContext::BaseDemoContext(ContextManager* pContextManager, DemoCore* pCore) :
GUIContext(pContextManager, pCore),
camSpeed(3), fastSpeedMultiplyer(8.5), ultraSpeedMultiplyer(30),
editorContext(pContextManager, pCore)
{
	currentSpeedMode = SpeedMode::NORMAL;

	isTrackingMouse = false;

	forwardMovement = 0;
	rightMovement = 0;

	//H rot: 4.10499
	//V rot: -0.275
	//X: -27.2224
	//Y: 21.0493
	//Z: 17.4667

	cam.SetFovY(gl::Degrees(70));
	cam.SetScreenWidth(pCore->GetScreenWidth());
	cam.SetScreenHeight(pCore->GetScreenHeight());
	cam.SetHorizontalRot(gl::Radians(4.10499));
	cam.SetVerticalRot(gl::Radians(-0.275));
	cam.SetPosition(gl::Vec3f(-27.2224, 21.0493, 17.4667));

	selectedCam = &cam;
	debugCam = cam;

	sunAngleRad = 2 * (15.f/180)*gl::math::Pi();
	
	pCore->GetGraphicsEngine().GetDebugDrawer().SetActiveCam(&debugCam);
	pCore->GetGraphicsEngine().SetActiveCamera(&cam);
	pCore->GetGraphicsEngine().SetActiveViewerCamera(&cam);
}

BaseDemoContext::~BaseDemoContext()
{}

void BaseDemoContext::HandleWindowEvent(const sf::Event& event)
{
	const int screenHeight = pCore->GetScreenHeight();
	const int screenWidth = pCore->GetScreenWidth();

	if (event.type == sf::Event::KeyPressed) {
		KeyPressed(event.key);
	}
	else if (event.type == sf::Event::KeyReleased) {
		KeyReleased(event.key);
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Right) {
			isTrackingMouse = true;
			pCore->GetWindow()->setMouseCursorVisible(!isTrackingMouse);
			sf::Mouse::setPosition(sf::Vector2i(screenWidth / 2, screenHeight / 2), *pCore->GetWindow());
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Button::Right) {
			isTrackingMouse = false;
			pCore->GetWindow()->setMouseCursorVisible(!isTrackingMouse);
		}
	}
	else if (event.type == sf::Event::Resized) {
		cam.SetScreenWidth(event.size.width);
		cam.SetScreenHeight(event.size.height);
	}
}

void BaseDemoContext::EnteringContext()
{
	cam.SetScreenWidth(pCore->GetScreenWidth());
	cam.SetScreenHeight(pCore->GetScreenHeight());
	debugCam.SetScreenWidth(pCore->GetScreenWidth());
	debugCam.SetScreenHeight(pCore->GetScreenHeight());
	pCore->GetGraphicsEngine().SetActiveCamera(&cam);
}

void BaseDemoContext::LeavingContext()
{
}

void BaseDemoContext::Update(float deltaSec)
{
	float currSpeed = GetCurrentSpeed();

	selectedCam->MoveForward(forwardMovement * currSpeed * deltaSec);
	selectedCam->MoveRight(rightMovement * currSpeed * deltaSec);

	sf::Vector2i sfCursorPos = sf::Mouse::getPosition(*pCore->GetWindow());

	if (isTrackingMouse) {
		const sf::Vector2i windowCenter = sf::Vector2i(pCore->GetScreenWidth() / 2, pCore->GetScreenHeight() / 2);

		selectedCam->RotateHorizontally(gl::Radians(-1.f*(sfCursorPos.x - windowCenter.x) * pCore->GetMouseSensitivity()));
		selectedCam->RotateVertically(gl::Radians(-1.f*(sfCursorPos.y - windowCenter.y) * pCore->GetMouseSensitivity()));

		if (sfCursorPos != windowCenter) {
			// Might be too agressive, but can't think of a better solution for now.
			sf::Mouse::setPosition(windowCenter, *pCore->GetWindow());
		}
	}

	pCore->GetGraphicsEngine().GetSun().SetDirectionTowardsSource(gl::Vec3f(1, 1, -1));
	//pCore->GetGraphicsEngine().GetSun().SetDirectionTowardsSource(gl::Vec3f(std::cos(sunAngleRad), std::sin(sunAngleRad), 0));
}

static gl::Mat4f MyTranspose(const gl::Mat4f& input)
{
	gl::Mat4f result;

	for (std::size_t i = 0; i != 4; ++i) {
		for (std::size_t j = 0; j != 4; ++j) {
			result.Set(i, j, input.At(j, i));
		}
	}

	return result;
}

void BaseDemoContext::Draw()
{
	assert(false);
}

void BaseDemoContext::DrawOverlayElements()
{}

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

void BaseDemoContext::KeyPressed(sf::Event::KeyEvent key)
{
	const int screenHeight = cam.GetScreenHeight();
	const int screenWidth = cam.GetScreenWidth();

	switch (key.code) {
	case sf::Keyboard::Escape:
		pCore->Stop();
		break;
	case sf::Keyboard::Z:
		isTrackingMouse = !isTrackingMouse;
		pCore->GetWindow()->setMouseCursorVisible(!isTrackingMouse);
		if (isTrackingMouse) {
			sf::Mouse::setPosition(sf::Vector2i(screenWidth / 2, screenHeight / 2), *pCore->GetWindow());
		}
		break;

	case sf::Keyboard::H:
		pCore->GetGraphicsEngine().SetWireframeModeEnabled(!(pCore->GetGraphicsEngine().GetWireframeModeEnabled()));
		//wireframeModeEnabled = !wireframeModeEnabled;
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

	case sf::Keyboard::F12:
		if (selectedCam == &cam) {
			selectedCam = &debugCam;
			std::cout << "Debug cam selected!" << std::endl;
		}
		else {
			selectedCam = &cam;
			std::cout << "Normal cam selected!" << std::endl;
		}
		break;

	case sf::Keyboard::Up:
		sunAngleRad += (15.f/180)*gl::math::Pi();
		break;

	case sf::Keyboard::Down:
		sunAngleRad -= (15.f/180)*gl::math::Pi();
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
