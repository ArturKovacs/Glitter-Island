#include "BaseDemoContext.hpp"

#include "DemoCore.hpp"

BaseDemoContext::BaseDemoContext(ContextManager* pContextManager, DemoCore* pCore) :
GUIContext(pContextManager, pCore),
camSpeed(3), fastSpeedMultiplyer(8.5), ultraSpeedMultiplyer(30),
terrainSize(500), waterLevel(49), terrain(pCore), water(terrainSize * 7),
depthMaterial(pCore),
editorContext(pContextManager, pCore)
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

	currentSpeedMode = SpeedMode::NORMAL;
	wireframeModeEnabled = false;

	isTrackingMouse = false;

	forwardMovement = 0;
	rightMovement = 0;

	//1 = x + x^2 + x^4 + x^16
	//x ~~ 0.569797
	//subfrustumFarPlanePositionRatios[0] = 1.23459e-4;
	//subfrustumFarPlanePositionRatios[1] = 0.105533159;
	//subfrustumFarPlanePositionRatios[2] = 0.430201759;
	//subfrustumFarPlanePositionRatios[3] = 1;

	subfrustumFarPlanePositionRatios[0] = 0.01;
	subfrustumFarPlanePositionRatios[1] = 0.04;
	subfrustumFarPlanePositionRatios[2] = 0.1;
	subfrustumFarPlanePositionRatios[3] = 0.3;
	subfrustumFarPlanePositionRatios[4] = 1;

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

	for (auto& currFramebuffer : lightCascadeShadowMapFramebuffers) {
		currFramebuffer.SetResolution(shadowMapResolution, shadowMapResolution);
	}
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
	pCore->SetActiveCamera(&cam);
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

	sun.SetDirectionTowardsSource(gl::Vec3f(1, 1, -1));
	//sun.SetDirectionTowardsSource(gl::Vec3f(std::cos(sunAngleRad), std::sin(sunAngleRad), 0));
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
	gl::Context& glContext = pCore->GetGLContext();

	if (wireframeModeEnabled) {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Line);
	}
	else {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	}
	glContext.Enable(gl::Capability::DepthTest);
	glContext.DepthFunc(gl::enums::CompareFunction::LEqual);

	gl::Vec3f lightViewZ = sun.GetDirectionTowardsSource();
	gl::Vec3f lightViewX = gl::Cross(gl::Vec3f(0, 1, 0), lightViewZ);
	if (gl::Dot(lightViewX, lightViewX) == 0) {
		lightViewX = gl::Vec3f(1, 0, 0);
	}
	else {
		lightViewX = gl::Normalized(lightViewX);
	}
	gl::Vec3f lightViewY = gl::Cross(lightViewZ, lightViewX);

	assert(lightViewX.y() == 0);
	assert(gl::Dot(lightViewX, lightViewY) < 0.001 && gl::Dot(lightViewZ, lightViewY) < 0.001 && gl::Dot(lightViewX, lightViewZ) < 0.001);
	//Use transpose instead of expensive inverse since this matrix is orthogonal.
	//TODO might be accidently written down transposed
	const gl::Mat4f lightViewTransform = (gl::Mat4f(
		lightViewX.x(), lightViewX.y(), lightViewX.z(), 0,
		lightViewY.x(), lightViewY.y(), lightViewY.z(), 0,
		lightViewZ.x(), lightViewZ.y(), lightViewZ.z(), 0,
		             0,              0,              0, 1));

	float prevZFar = cam.GetZNear();
	const float totalDepth = cam.GetZFar() - cam.GetZNear();
	for (int cascadeID = 0; cascadeID < lightCascadeCameras.size(); cascadeID++) {
		PerspectiveCamera subCamera = cam;
		subCamera.SetZNear(prevZFar);
		subCamera.SetZFar(cam.GetZNear() + totalDepth*subfrustumFarPlanePositionRatios[cascadeID]);

		Frustum subFrustum = subCamera.GetFrustum();
		subFrustum = subFrustum.Transformed(lightViewTransform);

		{
			using planeType = decltype(subFrustum.nearPlane);
			static_assert(std::is_same<gl::Vec3f, planeType::value_type >::value, "Error plane quad does not consist of expected types.");
		}
		gl::Vec3f AABBmin = subFrustum.nearPlane.at(0);
		gl::Vec3f AABBmax = subFrustum.nearPlane.at(0);

		assert(subFrustum.nearPlane.at(0).Size() == 3);

		for (auto& currVertex : subFrustum.nearPlane) {
			for (int i = 0; i < currVertex.Size(); i++) {
				AABBmin[i] = std::min(AABBmin[i], currVertex[i]);
				AABBmax[i] = std::max(AABBmax[i], currVertex[i]);
			}
		}
		for (auto& currVertex : subFrustum.farPlane) {
			for (int i = 0; i < currVertex.Size(); i++) {
				AABBmin[i] = std::min(AABBmin[i], currVertex[i]);
				AABBmax[i] = std::max(AABBmax[i], currVertex[i]);
			}
		}

		lightCascadeCameras[cascadeID].SetViewTransform(lightViewTransform);
		lightCascadeCameras[cascadeID].SetProjectionTransform(gl::CamMatrixf::Ortho(AABBmin.x(), AABBmax.x(), AABBmin.y(), AABBmax.y(), -AABBmax.z()-400, -AABBmin.z()+400));

		if (cascadeID <= 1) {
			pCore->GetDebugDrawer().SetActiveCam(&debugCam);
			PerspectiveCamera sub2 = cam;
			sub2.SetZNear(prevZFar);
			sub2.SetZFar(cam.GetZNear() + totalDepth*subfrustumFarPlanePositionRatios[cascadeID]);
			pCore->GetDebugDrawer().DrawOnce(sub2.GetFrustum());
			pCore->GetDebugDrawer().DrawOnce(lightCascadeCameras[cascadeID].GetFrustum());
		}
		prevZFar = subCamera.GetZFar();
	}

	//glContext.ClearColor(1, 1, 1, 1);
	for (int cascadeID = 0; cascadeID < lightCascadeCameras.size(); cascadeID++) {
		lightCascadeShadowMapFramebuffers[cascadeID].Bind(gl::enums::FramebufferTarget::Draw);
		glContext.DrawBuffer(gl::enums::ColorBuffer::None);

		pCore->SetActiveCamera(&(lightCascadeCameras[cascadeID]));
		glContext.Viewport(0, 0, shadowMapResolution, shadowMapResolution);
		glContext.Clear().DepthBuffer();
		//Draw depth only!
		DrawObjects();
	}

	pCore->GetCurrentFramebuffer().Bind(gl::enums::FramebufferTarget::Draw);
	pCore->SetActiveCamera(&cam);
	glContext.Viewport(0, 0, pCore->GetScreenWidth(), pCore->GetScreenHeight());
	glContext.ClearColor(0, 0, 0, 1);
	glContext.Clear().ColorBuffer().DepthBuffer();
	DrawObjects();

	skybox.Draw(*pCore);

	water.Draw(*pCore);

	//(TODO) WARNING: Drawing Skybox twice! It's only purpose is to make water fade out.
	//Please note that drawing skybox ONLY after water is not a good solution, because without a skybox behind water, it will refract black background making distant water black.
	skybox.Draw(*pCore);
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

int BaseDemoContext::GetLightCascadeCount() const
{
	return lightCascadeCount;
}

const gl::Texture& BaseDemoContext::GetCascadeShadowMap(int cascadeID) const
{
	return lightCascadeShadowMapFramebuffers.at(cascadeID).GetDepthTexture();
}

gl::Mat4f BaseDemoContext::GetCascadeViewProjectionTransform(int cascadeID) const
{
	return lightCascadeCameras.at(cascadeID).GetViewProjectionTransform();
}

float BaseDemoContext::GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const
{
	const float totalDepth = cam.GetZFar() - cam.GetZNear();
	gl::Vec4f farPlanePosInViewSpace = gl::Vec4f(0, 0, -1*(cam.GetZNear() + totalDepth*subfrustumFarPlanePositionRatios.at(subfrustumID)), 1);
	gl::Vec4f transformResult = cam.GetProjectionTransform() * farPlanePosInViewSpace;
	return (transformResult.z() / transformResult.w())*0.5f + 0.5f;
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

void BaseDemoContext::DrawObjects()
{
	gl::Mat4f projection = cam.GetProjectionTransform();
	gl::Mat4f view = cam.GetViewTransform();
	gl::Mat4f viewProjection = projection*view;

	pCore->GetGLContext().Enable(gl::Capability::CullFace);
	terrain.Draw();

	for (auto& current : graphicalObjects) {
		current.Draw(*pCore);
		//pCore->simpleColoredDrawer.Draw(pCore->GetGLContext(), current.GetMesh(), cam.GetViewProjectionTransform() * current.GetTransform(), gl::Vec4f(1, 1, 0, 0.7));
	}
}
