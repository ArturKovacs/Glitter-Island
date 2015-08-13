#include "DemoCore.hpp"

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <sstream>

#include "StandardMaterial.hpp"
#include "Utility.hpp"

const std::string DemoCore::shadersFolderPath = "../shaders/";
const std::string DemoCore::imgFolderPath = "../img/";
const std::string DemoCore::modelsFolderPath = "../models/";

gl::Program DemoCore::LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name)
{
	gl::Program result;

	gl::VertexShader vs;
	vs.Source(util::LoadFileAsString(shadersFolderPath + vs_name));
	try {
		vs.Compile();
	}
	catch (gl::Error& err) {
		throw std::runtime_error(std::string(err.what()) + "\n\nIn file: " + vs_name + "\n\nLog:\n" + err.Log());
	}

	gl::FragmentShader fs;
	fs.Source(util::LoadFileAsString(shadersFolderPath + fs_name));
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
running(false),
contextManager(this),
baseDemoContext(&contextManager, this),
pWindow(pWindow),
mouseSensitivity(0.005f),
pActiveCam(nullptr),
debugDrawer(this)
{
	//assert(pWindow->isActive())

	if (!overlayFont.loadFromFile("../font/Inconsolata.otf")) {
		throw std::runtime_error("Could not load font!");
	}

	circle = Mesh::GenerateCircle(1, 16);

	screenWidth = pWindow->getSize().x;
	screenHeight = pWindow->getSize().y;

	textDrawer.SetScreenResolution(gl::Vec2i(screenWidth, screenHeight));

	PushFramebuffer();

	finalFramebufferCopy = LoadShaderProgramFromFiles("FinalFramebufferCopy_v.glsl", "FinalFramebufferCopy_f.glsl");
	finalFramebufferCopy.Use();

	framebufferCopy_ScreenWidth = gl::Uniform<GLint>(finalFramebufferCopy, "screenWidth");
	framebufferCopy_ScreenHeight = gl::Uniform<GLint>(finalFramebufferCopy, "screenHeight");
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	//gl::UniformSampler(finalFramebufferCopy, "colorTex").Set(0);
	//gl::UniformSampler(finalFramebufferCopy, "depthTex").Set(1);

	/////////////////
	//Init more variables

	contextManager.PushContext(&baseDemoContext);


	//////////////////////////////////////////////////
	//TEST
	//////////////////////////////////////////////////

	gl::Mat4f viewTransform = gl::ModelMatrixf::RotationY(gl::Degrees(32.1));
	RawCamera testCam;
	testCam.SetViewTransform(viewTransform);
	testCam.SetProjectionTransform(gl::CamMatrixf::Ortho(-1, 1, -1, 1, 0, 1));
	Frustum frustum = testCam.GetFrustum();

	std::stringstream ss1;
	for (auto& currV : frustum.nearPlane) {
		ss1 << currV.x() << ", ";
		ss1 << currV.y() << ", ";
		ss1 << currV.z() << std::endl;
	}
	for (auto& currV : frustum.farPlane) {
		ss1 << currV.x() << ", ";
		ss1 << currV.y() << ", ";
		ss1 << currV.z() << std::endl;
	}

	std::stringstream ss2;
	testCam.SetViewTransform(gl::Mat4f());
	frustum = testCam.GetFrustum().Transformed(gl::Inverse(viewTransform));
	for (auto& currV : frustum.nearPlane) {
		ss2 << currV.x() << ", ";
		ss2 << currV.y() << ", ";
		ss2 << currV.z() << std::endl;
	}
	for (auto& currV : frustum.farPlane) {
		ss2 << currV.x() << ", ";
		ss2 << currV.y() << ", ";
		ss2 << currV.z() << std::endl;
	}

	assert(ss1.str() == ss2.str());

	//////////////////////////////////////////////////
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
	float avgFPS = 0;
	int framecount = 0;

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

			if (event.type == sf::Event::Resized) {
				Resize(event.size.width, event.size.height);
			}

			//ContextManagerWindowEvent(event);
			contextManager.HandleWindowEvent(event);
		}

		//ContextManagerUpdate(deltaSec);
		//ContextManagerDraw();

		currFPS = 1/deltaSec;

		contextManager.Update(deltaSec);
		CoreDraw();

		avgFPS = (avgFPS*framecount + currFPS*1) / (framecount + 1);
		framecount += 1;

		if (elapsedSec - lastMinResetSec > longestMinKeepSec) {
			recentMinFPS = 5000;
			lastMinResetSec = elapsedSec;
		}
		else {
			recentMinFPS = std::min(int(currFPS), recentMinFPS);
		}

		if (elapsedSec - lastFPSUpdateSec > FPSUpdateDelaySec) {
			int updatedFPS = (int)std::round(framesSinceLastFPSUpdate / (elapsedSec - lastFPSUpdateSec));
			pWindow->setTitle(sf::String("Glitter-Island <| FPS: ") + std::to_string(updatedFPS) + " current; " + std::to_string((int)avgFPS) + " avg; " + std::to_string(recentMinFPS) + " recent min |>");
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

void DemoCore::Stop()
{
	running = false;
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
	return baseDemoContext.GetWireframeModeEnabled();
}

int DemoCore::GetScreenWidth() const
{
	return screenWidth;
}

int DemoCore::GetScreenHeight() const
{
	return screenHeight;
}

void DemoCore::SetActiveCamera(Camera* cam)
{
	pActiveCam = cam;
}

Camera* DemoCore::GetActiveCamera()
{
	return pActiveCam;
}

DebugDrawer& DemoCore::GetDebugDrawer()
{
	return debugDrawer;
}

DirectionalLight& DemoCore::GetSun()
{
	return baseDemoContext.GetSun();
}

//Camera& DemoCore::GetCamera()
//{
//	return cam;
//}

Terrain& DemoCore::GetTerrain()
{
	return baseDemoContext.GetTerrain();
}

float DemoCore::GetMouseSensitivity()
{
	return mouseSensitivity;
}

void DemoCore::AddGraphicalObject(GraphicalObject&& newObject)
{
	baseDemoContext.AddGraphicalObject(std::move(newObject));
}

sf::Window* DemoCore::GetWindow()
{
	return pWindow;
}

int DemoCore::GetLightCascadeCount() const
{
	return baseDemoContext.GetLightCascadeCount();
}

const gl::Texture& DemoCore::GetCascadeShadowMap(int cascadeID) const
{
	return baseDemoContext.GetCascadeShadowMap(cascadeID);
}

gl::Mat4f DemoCore::GetCascadeViewProjectionTransform(int cascadeID) const
{
	return baseDemoContext.GetCascadeViewProjectionTransform(cascadeID);
}

float DemoCore::GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const
{
	return baseDemoContext.GetViewSubfrustumFarPlaneInTexCoordZ(subfrustumID);
}

Mesh* DemoCore::LoadMeshFromFile(const std::string& filename)
{
	return meshManager.LoadMeshFromFile(this, DemoCore::modelsFolderPath + filename);
}

Material* DemoCore::LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName)
{
	return materialManager.LoadStandardMaterialFromFile(this, DemoCore::modelsFolderPath + filename, materialName);
}

GraphicalObject DemoCore::LoadGraphicalObjectFromFile(const std::string& filename)
{
	GraphicalObject result;

	{
		Mesh* pMesh = LoadMeshFromFile(filename);

		result.SetMesh(pMesh);
	}

	return std::move(result);
}

void DemoCore::SaveAll()
{
	static int saveCount = 0;
	pWindow->setTitle("Saving...");

	baseDemoContext.GetTerrain().SaveMaterialMap();

	std::cout << saveCount++ << " Saved!" << std::endl;
}

////////////////////////////////////////////
//
// Private functions
//
////////////////////////////////////////////

void DemoCore::ClearFramebufferStack()
{
	framebuffers.resize(1);

	framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
}

void DemoCore::CoreDraw()
{
	glContext.Viewport(0, 0, screenWidth, screenHeight);
	ClearFramebufferStack();
	GetCurrentFramebuffer().Bind(gl::Framebuffer::Target::Draw);
	glContext.Disable(gl::Capability::Blend);
	glContext.Enable(gl::Capability::DepthTest);
	contextManager.Draw();

	//draw current framebuffer to screen
	GetCurrentFramebuffer().SetVertexPosName("vertexPos");
	GetCurrentFramebuffer().SetColorTexName("colorTex");
	GetCurrentFramebuffer().SetDepthTexName("depthTex");
	GetCurrentFramebuffer().SetShaderProgram(&finalFramebufferCopy);

	defaultFBO.Bind(gl::Framebuffer::Target::Draw);
	glContext.Clear().ColorBuffer().DepthBuffer();
	GetCurrentFramebuffer().Draw(*this);

	glContext.Viewport(0, 0, 200, 200);
	glContext.Enable(gl::Capability::ScissorTest);
	glContext.Scissor(0, 0, 200, 200);
	//GetCurrentFramebuffer().SetVertexPosName("vertexPos");
	//GetCurrentFramebuffer().SetColorTexName("colorTex");
	//GetCurrentFramebuffer().SetDepthTexName("depthTex");
	//GetCurrentFramebuffer().SetShaderProgram(&finalFramebufferCopy);
	//defaultFBO.Bind(gl::Framebuffer::Target::Draw);
	glContext.Clear().ColorBuffer().DepthBuffer();
	//GetCurrentFramebuffer().Draw(*this);
	//TODO draw debug drawer
	debugDrawer.Draw();
	glContext.Disable(gl::Capability::ScissorTest);
	glContext.Viewport(0, 0, screenWidth, screenHeight);

	glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	glContext.Disable(gl::Capability::DepthTest);
	glContext.Enable(gl::Capability::Blend);
	glContext.BlendFunc(gl::BlendFunction::SrcAlpha, gl::BlendFunction::OneMinusSrcAlpha);

	contextManager.DrawOverlayElements();
}

void DemoCore::Resize(const int width, const int height)
{
	screenWidth = width;
	screenHeight = height;
	GetGLContext().Viewport(0, 0, screenWidth, screenHeight);

	textDrawer.SetScreenResolution(gl::Vec2i(screenWidth, screenHeight));

	finalFramebufferCopy.Use();
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	for (auto& current : framebuffers) {
		current.SetResolution(width, height);
	}
}
