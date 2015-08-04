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
mouseSensitivity(0.005f)
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

	/////////////////
	//Init more variables

	//H rot: 4.10499
	//V rot: -0.275
	//X: -27.2224
	//Y: 21.0493
	//Z: 17.4667

	cam.SetFovY(gl::Degrees(70));
	cam.SetScreenWidth(screenWidth);
	cam.SetScreenHeight(screenHeight);
	cam.SetHorizontalRot(gl::Radians(4.10499));
	cam.SetVerticalRot(gl::Radians(-0.275));
	cam.SetPosition(gl::Vec3f(-27.2224, 21.0493, 17.4667));

	contextManager.PushContext(&baseDemoContext);
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

		if (elapsedSec - lastMinResetSec > longestMinKeepSec) {
			recentMinFPS = 5000;
			lastMinResetSec = elapsedSec;
		}
		else {
			recentMinFPS = std::min(int(currFPS), recentMinFPS);
		}

		if (elapsedSec - lastFPSUpdateSec > FPSUpdateDelaySec) {
			int updatedFPS = (int)std::round(framesSinceLastFPSUpdate / (elapsedSec - lastFPSUpdateSec));
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

DirectionalLight& DemoCore::GetSun()
{
	return baseDemoContext.GetSun();
}

Camera& DemoCore::GetCamera()
{
	return cam;
}

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

Mesh* DemoCore::LoadMeshFromFile(const std::string& filename)
{
	auto elementIter = meshes.find(filename);
	if (elementIter != meshes.end()) {
		return (*elementIter).second;
	}

	Mesh* pResult = new Mesh;

	pResult->LoadFromOBJFile(this, DemoCore::modelsFolderPath + filename);

	meshes[filename] = pResult;

	return pResult;
}

Material* DemoCore::LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName)
{
	const std::string materialKey = filename + "?" + materialName;

	auto elementIter = materials.find(materialKey);
	if (elementIter != materials.end()) {
		return (*elementIter).second;
	}

	StandardMaterial *pResult = new StandardMaterial(this);

	std::cout << "Loading material..." << std::endl;

	pResult->LoadFromMTLFile(DemoCore::modelsFolderPath + filename, materialName);

	std::cout << "Material loaded!" << std::endl;

	materials[filename] = pResult;

	return pResult;
}

GraphicalObject DemoCore::LoadGraphicalObjectFromFile(const std::string& filename)
{
	GraphicalObject result;

	{
		Mesh* pMesh = LoadMeshFromFile(filename);

		result.SetMesh(pMesh);
	}

	/*
	const std::string materialFilename = filename.substr(0, filename.rfind(".obj")) + ".mtl";

	try {
		result.SetMaterial(LoadStandardMaterialFromFile(materialFilename));
		//result.LoadMaterial(DemoCore::modelsFolderPath + materialFilename);
	}
	catch (std::runtime_error& ex) {
		std::cout << "Exception occured while loading material for: " << filename << std::endl;
		std::cout << "Message: " << ex.what() << std::endl;
	}*/

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
	while (framebuffers.size() > 1) {
		framebuffers.pop_back();
	}

	framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
}

void DemoCore::CoreDraw()
{
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

	glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	glContext.Disable(gl::Capability::DepthTest);
	glContext.Enable(gl::Capability::Blend);
	glContext.BlendFunc(gl::BlendFunction::SrcAlpha, gl::BlendFunction::OneMinusSrcAlpha);

	contextManager.DrawOverlayElements();
}

/*
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
*/

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
