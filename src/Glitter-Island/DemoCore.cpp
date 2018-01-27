#include "DemoCore.hpp"

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>

//#include <GE/StandardMaterial.hpp>
#include <GE/Utility.hpp>

const std::string DemoCore::shadersFolderPath = "../shaders/";
const std::string DemoCore::imgFolderPath = "../img/";
const std::string DemoCore::modelsFolderPath = "../models/";

DemoCore::DemoCore(sf::Window* pWindow) :
running(false),
contextManager(this),
baseDemoContext(&contextManager, this),
pWindow(pWindow),
mouseSensitivity(0.005f)
{
	//assert(pWindow->isActive())

	if (!overlayFont.loadFromFile("../font/Inconsolata.otf")) {
		throw std::runtime_error("Could not load font!");
	}

	circle = Mesh::GenerateCircle(1, 16);

	graphicsEngine.Resize(pWindow->getSize().x, pWindow->getSize().y);

	textDrawer.SetScreenResolution(glm::ivec2(graphicsEngine.GetScreenWidth(),
                graphicsEngine.GetScreenHeight()));

	contextManager.PushContext(&baseDemoContext);
	
	graphicsEngine.GetShadersFolderPath() = shadersFolderPath;
	graphicsEngine.GetImgFolderPath() = imgFolderPath;
	graphicsEngine.GetModelsFolderPath() = modelsFolderPath;

	//////////////////////////////////////////////////
	// TEST
	//////////////////////////////////////////////////

	glm::mat4 viewTransform = glm::rotate(glm::mat4(1.f), glm::radians(32.1f), glm::vec3(0, 1, 0));
	RawCamera testCam;
	testCam.SetViewTransform(viewTransform);
	testCam.SetProjectionTransform(glm::ortho<float>(-1, 1, -1, 1, 0, 1));
	Frustum frustum = testCam.GetFrustum();

	std::stringstream ss1;
	for (auto& currV : frustum.nearPlane) {
		ss1 << currV.x << ", ";
		ss1 << currV.y << ", ";
		ss1 << currV.z << std::endl;
	}
	for (auto& currV : frustum.farPlane) {
		ss1 << currV.x << ", ";
		ss1 << currV.y << ", ";
		ss1 << currV.z << std::endl;
	}

	std::stringstream ss2;
	testCam.SetViewTransform(glm::mat4());
	frustum = testCam.GetFrustum().Transformed(glm::inverse(viewTransform));
	for (auto& currV : frustum.nearPlane) {
		ss2 << currV.x << ", ";
		ss2 << currV.y << ", ";
		ss2 << currV.z << std::endl;
	}
	for (auto& currV : frustum.farPlane) {
		ss2 << currV.x << ", ";
		ss2 << currV.y << ", ";
		ss2 << currV.z << std::endl;
	}

	assert(ss1.str() == ss2.str());

	//////////////////////////////////////////////////
}

int DemoCore::Start()
{
	running = true;

	pWindow->setKeyRepeatEnabled(false);

	auto& glContext = graphicsEngine.GetGLContext();
	
	glContext.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glContext.ClearDepth(1.0f);
	glContext.Enable(gl::enums::Capability::DepthTest);
	glContext.Enable(gl::enums::Capability::CullFace);

	double lastFPSUpdateSec = 0;
	double FPSUpdateDelaySec = 0.1;

	int framesSinceLastFPSUpdate = 0;
	int recentMinFPS = std::numeric_limits<int>::max();
	float avgFPS = 0;
	int framecount = 0;

	double lastMinResetSec = 0;
	double longestMinKeepSec = 7;

	float lastUpdateSec = clock.getElapsedTime().asSeconds()-1;

	while (running) {
		elapsedSec = clock.getElapsedTime().asSeconds();
		float deltaSec = elapsedSec - lastUpdateSec;
		lastUpdateSec = elapsedSec;

		sf::Event event;
		while (pWindow->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running = false;
			}

			if (event.type == sf::Event::Resized) {
				textDrawer.SetScreenResolution(
				glm::ivec2(event.size.width, event.size.height));
				
				graphicsEngine.Resize(event.size.width, event.size.height);
			}

			contextManager.HandleWindowEvent(event);
		}

		currFPS = 1/deltaSec;

		contextManager.Update(deltaSec);
		graphicsEngine.Update(elapsedSec);
		graphicsEngine.Draw();
		DrawOverlayElements();

		avgFPS = (avgFPS*framecount + currFPS*1) / (framecount + 1);
		framecount += 1;

		if (elapsedSec - lastMinResetSec > longestMinKeepSec) {
			recentMinFPS = std::numeric_limits<int>::max();
			lastMinResetSec = elapsedSec;
		}
		else {
			recentMinFPS = std::min(int(currFPS), recentMinFPS);
		}

		framesSinceLastFPSUpdate++;
		if (elapsedSec - lastFPSUpdateSec > FPSUpdateDelaySec) {
			int updatedFPS = int(std::round(framesSinceLastFPSUpdate / (elapsedSec - lastFPSUpdateSec)));
			std::stringstream ss;
			ss << "Glitter-Island -- FPS: " << std::setw(3) << updatedFPS << " current; " << std::setw(3) << (int)avgFPS << " avg; " << std::setw(3) << recentMinFPS << " recent min -- Build Date: " __DATE__;
			pWindow->setTitle(ss.str());
			lastFPSUpdateSec = elapsedSec;
			framesSinceLastFPSUpdate = 0;
		}

		pWindow->display();
	}

	return EXIT_SUCCESS;
}

void DemoCore::Stop()
{
	running = false;
}

sf::Time DemoCore::GetElapsedTime()
{
	return sf::seconds(elapsedSec);
}

int DemoCore::GetScreenWidth() const
{
	return graphicsEngine.GetScreenWidth();
}

int DemoCore::GetScreenHeight() const
{
	return graphicsEngine.GetScreenHeight();
}

float DemoCore::GetMouseSensitivity()
{
	return mouseSensitivity;
}

GraphicsEngine& DemoCore::GetGraphicsEngine()
{
	return graphicsEngine;
}

sf::Window* DemoCore::GetWindow()
{
	return pWindow;
}

void DemoCore::SaveAll()
{
	static int saveCount = 0;
	pWindow->setTitle("Saving...");

	graphicsEngine.GetTerrain().SaveMaterialMap();

	std::cout << saveCount++ << " Saved!" << std::endl;
}

////////////////////////////////////////////
//
// Private functions
//
////////////////////////////////////////////

void DemoCore::DrawOverlayElements()
{
	gl::Context& glContext = graphicsEngine.GetGLContext();

	glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	glContext.Disable(gl::Capability::DepthTest);
	glContext.Enable(gl::Capability::Blend);
	glContext.BlendFunc(gl::BlendFunction::SrcAlpha, gl::BlendFunction::OneMinusSrcAlpha);

	contextManager.DrawOverlayElements();
}
