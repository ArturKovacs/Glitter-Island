#include <GE/GraphicsEngine.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <GE/Utility.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>

std::string GraphicsEngine::shadersFolderPath = "../shaders/";
std::string GraphicsEngine::imgFolderPath = "../img/";
std::string GraphicsEngine::modelsFolderPath = "../models/";

//std::string GraphicsEngine::imgFolderPath;
//std::string GraphicsEngine::modelsFolderPath;
//std::string GraphicsEngine::shadersFolderPath;

#define IGNORE_TRY(x) try{x;}catch(std::exception&ex){std::cout<<ex.what()<<std::endl;}
#define RAND_01 (double(std::rand())/RAND_MAX)

std::string& GraphicsEngine::GetShadersFolderPath()
{
	return shadersFolderPath;
}

std::string& GraphicsEngine::GetImgFolderPath()
{
	return imgFolderPath;
}

std::string& GraphicsEngine::GetModelsFolderPath()
{
	return modelsFolderPath;
}

gl::Program GraphicsEngine::LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name, const std::map<std::string, std::string>& host_defined)
{
	gl::Program result;

	std::string fileContent = util::LoadFileAsString(shadersFolderPath + vs_name);
	gl::VertexShader vs;
	try {
		ParseHostDefinitions(fileContent, host_defined);
		vs.Source(fileContent);
		vs.Compile();
	}
	catch (gl::Error& err) {
		throw std::runtime_error(std::string(err.what()) + "\n\nIn file: " + vs_name + "\n\nLog:\n" + err.Log());
	}

	fileContent = util::LoadFileAsString(shadersFolderPath + fs_name);
	gl::FragmentShader fs;
	try {
		ParseHostDefinitions(fileContent, host_defined);
		fs.Source(fileContent);
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

GraphicsEngine::GraphicsEngine() :
screenWidth(0), screenHeight(0),
terrainSize(500), waterLevel(49), terrain(this), water(this, terrainSize * 7),
pActiveCam(nullptr), pActiveViewerCam(nullptr),
debugDrawer(this),
skybox(this)
{
	//terrain.LoadFromHeightMap(DemoCore::imgFolderPath + "heightMap.png", terrainSize, 0.06);
	terrain.LoadFromHeightMap(GetImgFolderPath() + "heightMap.png", terrainSize, 0.2f);

	glm::mat4 transform = glm::rotate(glm::mat4(1), glm::half_pi<float>(), glm::vec3(0, 1, 0));
	transform = glm::rotate(glm::translate(transform, glm::vec3(-terrainSize*0.5, -waterLevel, terrainSize*0.5)), -glm::pi<float>() / 2.f, glm::vec3(1, 0, 0));
	terrain.SetTransform(transform);
	

	skybox.LoadTextureFromFiles(
		GetImgFolderPath() + "sb4-x.bmp",
		GetImgFolderPath() + "sb1+x.bmp",
		GetImgFolderPath() + "sb6-y.bmp",
		GetImgFolderPath() + "sb3+y.bmp",
		GetImgFolderPath() + "sb2-z.bmp",
		GetImgFolderPath() + "sb5+z.bmp");

	sun.SetColor(glm::vec3(1, 1, 1));
	
	reducedSizeFramebufferScale = 0.75;

	intermediateFramebuffer = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	aoResultFB = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	objectsFB = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH | Framebuffer::ATTACHMENT_NORMAL | Framebuffer::ATTACHMENT_VIEW_DEPTH);
	reducedSizeIntermFramebuffer1 = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	reducedSizeIntermFramebuffer2 = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	AddFramebufferForManagment(intermediateFramebuffer);
	AddFramebufferForManagment(aoResultFB);
	AddFramebufferForManagment(objectsFB);
	AddFramebufferForManagment(reducedSizeIntermFramebuffer1, reducedSizeFramebufferScale, reducedSizeFramebufferScale);
	AddFramebufferForManagment(reducedSizeIntermFramebuffer2, reducedSizeFramebufferScale, reducedSizeFramebufferScale);

	finalFramebufferCopy = LoadShaderProgramFromFiles("FinalFramebufferCopy_v.glsl", "FinalFramebufferCopy_f.glsl");
	finalFramebufferCopy.Use();
	framebufferCopy_ScreenWidth = gl::Uniform<GLint>(finalFramebufferCopy, "screenWidth");
	framebufferCopy_ScreenHeight = gl::Uniform<GLint>(finalFramebufferCopy, "screenHeight");
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	ssaoCalcProgram = LoadShaderProgramFromFiles("Passthrough2_v.glsl", "SSAO_Calc_f.glsl");
	ssaoCalcProgram.Use();
	ssaoCalc_proj = gl::Uniform<glm::mat4>(ssaoCalcProgram, "proj");
	ssaoCalc_projInv = gl::Uniform<glm::mat4>(ssaoCalcProgram, "projInv");
	IGNORE_TRY( ssaoCalc_viewTrInv = gl::Uniform<glm::mat4>(ssaoCalcProgram, "viewTrInv") );
	
	IGNORE_TRY(ssaoCalc_screenWidth = gl::Uniform<GLint>(ssaoCalcProgram, "screenWidth"));
	IGNORE_TRY(ssaoCalc_screenHeight = gl::Uniform<GLint>(ssaoCalcProgram, "screenHeight"));

	ssaoBlurHorProg = LoadShaderProgramFromFiles("Passthrough2_v.glsl", "SSAO_BlurHor_f.glsl");
	ssaoBlurHorProg.Use();
	IGNORE_TRY(ssaoBlurHor_screenWidth = gl::Uniform<GLint>(ssaoBlurHorProg, "screenWidth"));
	IGNORE_TRY(ssaoBlurHor_screenHeight = gl::Uniform<GLint>(ssaoBlurHorProg, "screenHeight"));

	ssaoBlurVerProg = LoadShaderProgramFromFiles("Passthrough2_v.glsl", "SSAO_BlurVer_f.glsl");
	ssaoBlurVerProg.Use();
	IGNORE_TRY(ssaoBlurVer_screenWidth = gl::Uniform<GLint>(ssaoBlurVerProg, "screenWidth"));
	IGNORE_TRY(ssaoBlurVer_screenHeight = gl::Uniform<GLint>(ssaoBlurVerProg, "screenHeight"));

	ssaoDrawProgram = LoadShaderProgramFromFiles("Passthrough2_v.glsl", "SSAO_Draw_f.glsl");
	ssaoDrawProgram.Use();
	IGNORE_TRY(ssaoDraw_screenWidth = gl::Uniform<GLint>(ssaoDrawProgram, "screenWidth"));
	IGNORE_TRY(ssaoDraw_screenHeight = gl::Uniform<GLint>(ssaoDrawProgram, "screenHeight"));

	constexpr double aoRadius = 0.25;
	for (auto& currSample : sampleVecs) {
		bool valid;
		const double PI = glm::pi<double>();
		do {
			double theta = RAND_01 * 2 * PI;
			double phi = RAND_01 * PI - PI/2;
			currSample = glm::vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
			valid = glm::dot(currSample, glm::vec3(1, 0, 0)) > 0.01f;
			if (valid) {
				double radiusFactor = std::max(RAND_01, 0.2);
				radiusFactor = std::pow(radiusFactor, 3);
				currSample *= aoRadius * radiusFactor;
			}
		} while (!valid);
	}

	ssaoCalcProgram.Use();
	for (size_t i = 0; i < ssaoCalc_sampleVecs.size(); i++) {
		IGNORE_TRY( ssaoCalc_sampleVecs[i] = gl::Uniform<glm::vec3>(ssaoCalcProgram, "sampleVecs["+std::to_string(i)+"]") );
		IGNORE_TRY( ssaoCalc_sampleVecs[i].Set(sampleVecs[i]) );
	}

	std::array<glm::vec2, 4*4> noiseArray;
	for (auto& curr : noiseArray) {
		curr = glm::vec2(RAND_01*2-1, RAND_01*2-1);
	}
	noise4.Bind(gl::TextureTarget::_2D);
	gl::Texture::Image2D(gl::TextureTarget::_2D, 0, gl::PixelDataInternalFormat::RG16F, 4, 4, 0, gl::PixelDataFormat::RG, gl::DataType::Float, noiseArray.data());
	gl::Texture::MinFilter(gl::TextureTarget::_2D, gl::enums::TextureMinFilter::Nearest);
	gl::Texture::MagFilter(gl::TextureTarget::_2D, gl::enums::TextureMagFilter::Nearest);
	gl::Texture::WrapS(gl::TextureTarget::_2D, gl::enums::TextureWrap::Repeat);
	gl::Texture::WrapT(gl::TextureTarget::_2D, gl::enums::TextureWrap::Repeat);

	debugDrawer.SetEnabled(false);
	wireframeModeEnabled = false;
	
	//1 = x + x^2 + x^4 + x^16
	//x ~~ 0.569797
	//subfrustumFarPlanePositionRatios[0] = 1.23459e-4;
	//subfrustumFarPlanePositionRatios[1] = 0.105533159;
	//subfrustumFarPlanePositionRatios[2] = 0.430201759;
	//subfrustumFarPlanePositionRatios[3] = 1;
	
	subfrustumFarPlanePositionRatios[0] = 0.01f;
	subfrustumFarPlanePositionRatios[1] = 0.04f;
	subfrustumFarPlanePositionRatios[2] = 0.1f;
	subfrustumFarPlanePositionRatios[3] = 0.3f;
	subfrustumFarPlanePositionRatios[4] = 1.f;
	
	for (auto& currFramebuffer : lightCascadeShadowMapFramebuffers) {
		currFramebuffer = std::move(Framebuffer{shadowMapResX, shadowMapResY, Framebuffer::ATTACHMENT_DEPTH});
		//currFramebuffer.SetResolution(shadowMapResolution, shadowMapResolution);
	}
}

void GraphicsEngine::Update(double elapsedSeconds)
{
	elapsedSec = elapsedSeconds;
}

void GraphicsEngine::Draw()
{
	glContext.Viewport(0, 0, screenWidth, screenHeight);
	//intermediateFramebuffer.Bind(gl::Framebuffer::Target::Draw);
	//glContext.Disable(gl::Capability::Blend);
	//glContext.Enable(gl::Capability::DepthTest);
	DrawScene();

	//draw current framebuffer to screen
	GetCurrentFramebuffer().SetVertexPosName("vertexPos");
	GetCurrentFramebuffer().SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "colorTex", 0);
	GetCurrentFramebuffer().SetTextureShaderID(Framebuffer::ATTACHMENT_DEPTH, "depthTex", 1);
	GetCurrentFramebuffer().SetShaderProgram(&finalFramebufferCopy, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);

	defaultFBO.Bind(gl::Framebuffer::Target::Draw);
	glContext.Clear().ColorBuffer().DepthBuffer();
	GetCurrentFramebuffer().Draw(*this);

	glContext.Viewport(0, 0, 200, 200);
	glContext.Enable(gl::Capability::ScissorTest);
	glContext.Scissor(0, 0, 200, 200);
	debugDrawer.Draw();
	glContext.Disable(gl::Capability::ScissorTest);
	glContext.Viewport(0, 0, screenWidth, screenHeight);
}

double GraphicsEngine::GetElapsedSeconds() const
{
	return elapsedSec;
}

void GraphicsEngine::Resize(const int width, const int height)
{
	screenWidth = width;
	screenHeight = height;
	GetGLContext().Viewport(0, 0, screenWidth, screenHeight);

	//textDrawer.SetScreenResolution(glm::ivec2(screenWidth, screenHeight));
	
	finalFramebufferCopy.Use();
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	for (const auto& current : managedFramebuffers) {
		current.pFramebuffer->SetResolution(int(width*current.scaleX), int(height*current.scaleY));
	}

	//halfSizedIntermFramebuffer1.SetResolution(width/2, height/2);
	//halfSizedIntermFramebuffer2.SetResolution(width/2, height/2);
}

int GraphicsEngine::GetScreenWidth() const
{
	return screenWidth;
}

int GraphicsEngine::GetScreenHeight() const
{
	return screenHeight;
}

void GraphicsEngine::SetCurrentFramebuffer(Framebuffer& current)
{
	current.Bind(gl::FramebufferTarget::Draw);
	pCurrentFramebuffer = &current;
}

Framebuffer& GraphicsEngine::GetCurrentFramebuffer()
{
	return *pCurrentFramebuffer;
}

Framebuffer& GraphicsEngine::GetIntermediateFramebuffer()
{
	return intermediateFramebuffer;
}

void GraphicsEngine::AddFramebufferForManagment(Framebuffer& framebuffer)
{
	AddFramebufferForManagment(framebuffer, 1, 1);
}

void GraphicsEngine::AddFramebufferForManagment(Framebuffer& framebuffer, float scaleX, float scaleY)
{
	FramebufferRegister newRegister;
	newRegister.pFramebuffer = &framebuffer;
	newRegister.scaleX = scaleX;
	newRegister.scaleY = scaleY;
	managedFramebuffers.push_back(newRegister);
}

float GraphicsEngine::GetObjectsDepthBufferValue(int x, int y)
{
	objectsFB.Bind(gl::FramebufferTarget::Read);
	GLfloat result;
	glContext.ReadPixels(x, y, 1, 1, gl::enums::PixelDataFormat::DepthComponent, gl::PixelDataType::Float, &result);
	//gl::Framebuffer::Bind(gl::FramebufferTarget::Read, gl::FramebufferName::InvalidName());
	
	//RESET framebuffer that was bound if framebuffer was needed to be bound on draw target
	//SetCurrentFramebuffer(GetCurrentFramebuffer());
	
	return result;
}

gl::Context& GraphicsEngine::GetGLContext()
{
	return glContext;
}

void GraphicsEngine::SetWireframeModeEnabled(bool enabled)
{
	wireframeModeEnabled = enabled;
}

bool GraphicsEngine::GetWireframeModeEnabled() const
{
	return wireframeModeEnabled;
}

DirectionalLight& GraphicsEngine::GetSun()
{
	return sun;
}

Terrain& GraphicsEngine::GetTerrain()
{
	return terrain;
}

Water& GraphicsEngine::GetWater()
{
	return water;
}

Skybox& GraphicsEngine::GetSkybox()
{
	return skybox;
}

void GraphicsEngine::SetActiveViewerCamera(PerspectiveCamera* cam)
{
	pActiveViewerCam = cam;
}

PerspectiveCamera* GraphicsEngine::GetActiveViewerCamera()
{
	return pActiveViewerCam;
}

Camera* GraphicsEngine::GetActiveCamera()
{
	return pActiveCam;
}

int GraphicsEngine::GetLightCascadeCount() const
{
	return lightCascadeCount;
}

const gl::Texture& GraphicsEngine::GetCascadeShadowMap(int cascadeID) const
{
	return lightCascadeShadowMapFramebuffers.at(cascadeID).GetTexture(Framebuffer::ATTACHMENT_DEPTH);
}

glm::mat4 GraphicsEngine::GetCascadeViewProjectionTransform(int cascadeID) const
{
	return lightCascadeCameras.at(cascadeID).GetViewProjectionTransform();
}

float GraphicsEngine::GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const
{
	const float totalDepth = pActiveViewerCam->GetZFar() - pActiveViewerCam->GetZNear();
	glm::vec4 farPlanePosInViewSpace = glm::vec4(0, 0, -1*(pActiveViewerCam->GetZNear() + totalDepth*subfrustumFarPlanePositionRatios.at(subfrustumID)), 1);
	glm::vec4 transformResult = pActiveViewerCam->GetProjectionTransform() * farPlanePosInViewSpace;
	return (transformResult.z / transformResult.w)*0.5f + 0.5f;
}

util::managed_ptr<Mesh> GraphicsEngine::LoadMeshFromFile(const std::string& filename)
{
	return meshManager.LoadMeshFromFile(this, GraphicsEngine::modelsFolderPath + filename);
}

util::managed_ptr<Material> GraphicsEngine::LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName)
{
	return materialManager.LoadFromFile(this, GraphicsEngine::modelsFolderPath + filename, materialName);
}

util::managed_ptr<GraphicalObject> GraphicsEngine::CreateGraphicalObject()
{
	managedGraphicalObjects.push_back(StandardGraphicalObject());
	GraphicalObject* pResult = &managedGraphicalObjects.back();

	return pResult;
}

DebugDrawer& GraphicsEngine::GetDebugDrawer()
{
	return debugDrawer;
}

SimpleColoredDrawer& GraphicsEngine::GetSimpleColoredDrawer()
{
	return simpleColoredDrawer;
}

////////////////////////////////////////////
//
// Private functions
//
////////////////////////////////////////////

void GraphicsEngine::ParseHostDefinitions(std::string& shader_src, const std::map<std::string, std::string>& host_defined)
{
	bool found;
	do {
		const std::string prefix = "\n+host_defined";
		size_t expr_start = shader_src.find(prefix);
		found = expr_start != std::string::npos;
		if (found) {
			size_t begin = shader_src.find_first_not_of(" \t", expr_start + prefix.length());
			std::string identifier = shader_src.substr(begin, shader_src.find('\n', begin) - begin);
			std::string value;
			try {
				value = host_defined.at(identifier);
			}
			catch (std::out_of_range&) {
				throw std::runtime_error("Could not find " + identifier + " among given host definitions while parsing shader");
			}

			std::string replacement = "\n#define " + identifier + " " + value;
			size_t expr_end = shader_src.find('\n', expr_start+1);
			shader_src.replace(expr_start, expr_end-expr_start, replacement);
		}
	} while (found);
}

void GraphicsEngine::SetActiveCamera(Camera* cam)
{
	pActiveCam = cam;
}

void GraphicsEngine::UpdateLightViewTransform()
{
	glm::vec3 lightViewZ = sun.GetDirectionTowardsSource();
	glm::vec3 lightViewX = glm::cross(glm::vec3(0, 1, 0), lightViewZ);
	if (glm::dot(lightViewX, lightViewX) == 0) {
		lightViewX = glm::vec3(1, 0, 0);
	}
	else {
		lightViewX = glm::normalize(lightViewX);
	}
	glm::vec3 lightViewY = glm::cross(lightViewZ, lightViewX);

	assert(lightViewX.y == 0);
	assert(glm::dot(lightViewX, lightViewY) < 0.001 && glm::dot(lightViewZ, lightViewY) < 0.001 && glm::dot(lightViewX, lightViewZ) < 0.001);

	/// Use transpose instead of expensive inverse since this matrix is orthogonal.
	/// The elements are written down transposed so this matrix
	/// is already transposed when constructed.
	lightViewTransform = glm::mat4(
		lightViewX.x, lightViewY.x, lightViewZ.x, 0,
		lightViewX.y, lightViewY.y, lightViewZ.y, 0,
		lightViewX.z, lightViewY.z, lightViewZ.z, 0,
		           0,            0,            0, 1);
}

float GraphicsEngine::GetSubfrustumZNear(int cascadeID) const
{
	const float totalDepth = pActiveViewerCam->GetZFar() - pActiveViewerCam->GetZNear();
	const int prevID = cascadeID-1;
	float prevFarPlaneRatio = prevID >=0 ? subfrustumFarPlanePositionRatios[prevID] : 0;
	return pActiveViewerCam->GetZNear() + totalDepth*prevFarPlaneRatio;
}

void GraphicsEngine::UpdateLightCascadeCamera(int cascadeID)
{
	const float totalDepth = pActiveViewerCam->GetZFar() - pActiveViewerCam->GetZNear();
	PerspectiveCamera subCamera = *pActiveViewerCam;
	subCamera.SetZNear(GetSubfrustumZNear(cascadeID));
	subCamera.SetZFar(pActiveViewerCam->GetZNear() + totalDepth*subfrustumFarPlanePositionRatios[cascadeID]);

	Frustum subFrustum = subCamera.GetFrustum();
	subFrustum = subFrustum.Transformed(lightViewTransform);

	glm::vec3 AABBmin = subFrustum.nearPlane.at(0);
	glm::vec3 AABBmax = subFrustum.nearPlane.at(0);
	
	for (auto& currVertex : subFrustum.nearPlane) {
		AABBmin = glm::min(AABBmin, currVertex);
		AABBmax = glm::max(AABBmax, currVertex);
	}
	for (auto& currVertex : subFrustum.farPlane) {
		AABBmin = glm::min(AABBmin, currVertex);
		AABBmax = glm::max(AABBmax, currVertex);
	}

	lightCascadeCameras[cascadeID].SetViewTransform(lightViewTransform);
	lightCascadeCameras[cascadeID].SetProjectionTransform(glm::ortho(AABBmin.x, AABBmax.x, AABBmin.y, AABBmax.y, -AABBmax.z-400, -AABBmin.z+400));

	if (cascadeID <= 1) {
		PerspectiveCamera sub2 = *pActiveViewerCam;
		sub2.SetZNear(GetSubfrustumZNear(cascadeID));
		sub2.SetZFar(pActiveViewerCam->GetZNear() + totalDepth*subfrustumFarPlanePositionRatios[cascadeID]);
		debugDrawer.DrawOnce(sub2.GetFrustum());
		debugDrawer.DrawOnce(lightCascadeCameras[cascadeID].GetFrustum());
	}
}

void GraphicsEngine::DrawShadowMap(int cascadeID)
{
	//lightCascadeShadowMapFramebuffers[cascadeID].Bind(gl::enums::FramebufferTarget::Draw);
	SetCurrentFramebuffer(lightCascadeShadowMapFramebuffers[cascadeID]);

	//Draw depth only!
	glContext.DrawBuffer(gl::enums::ColorBuffer::None);

	SetActiveCamera(&(lightCascadeCameras[cascadeID]));
	glContext.Viewport(0, 0, shadowMapResX, shadowMapResY);
	glContext.Clear().DepthBuffer();
	glContext.CullFace(gl::enums::Face::Front);

	DrawObjects();

	glContext.CullFace(gl::enums::Face::Back);
}

void GraphicsEngine::DrawObjects()
{
	//Update instanced object list
	for (auto& currObject : managedGraphicalObjects) {
		for (auto& currSubmesh : currObject.GetMesh()->GetSubmeshes()) {
			instancedGraphicalObjects[&currSubmesh].insert(&currObject);
		}
	}

	//glContext.Disable(gl::Capability::CullFace);
	//glContext.Disable(gl::Capability::Blend);

	for (auto& currAssociation : instancedGraphicalObjects) {
		Mesh::Submesh* submesh = currAssociation.first;
		Material* pMaterial = submesh->GetMaterial();
		if (pMaterial != nullptr) {
			submesh->BindVAO();
			auto& objectsWithThisSubmesh = currAssociation.second;
			pMaterial->Prepare(*submesh);
			for(auto currentObject : objectsWithThisSubmesh) {
				if (currentObject->IsVisible()){
					if (currentObject->IsDepthTestEnabled()) {
						glContext.Enable(gl::Capability::DepthTest);
					} else {
						glContext.Disable(gl::Capability::DepthTest);
					}
					pMaterial->SetTransform(currentObject->GetTransform());
					glContext.DrawElements(submesh->GetPrimitiveType(), submesh->GetNumOfIndices(), submesh->indexTypeEnum);
				}
			}
		}
	}

	//glContext.Disable(gl::Capability::Blend);
	//glContext.Enable(gl::Capability::CullFace);
}

void GraphicsEngine::DrawScene()
{
	if (wireframeModeEnabled) {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Line);
	}
	else {
		glContext.PolygonMode(gl::enums::Face::FrontAndBack, gl::PolygonMode::Fill);
	}
	glContext.Enable(gl::Capability::DepthTest);
	glContext.DepthFunc(gl::enums::CompareFunction::LEqual);

	UpdateLightViewTransform();

	static int cascadeID = 0;
	for (int i = 0; i < 1; i ++) {
		UpdateLightCascadeCamera(cascadeID);
		DrawShadowMap(cascadeID);
		cascadeID += 1;
		cascadeID = cascadeID % lightCascadeCount;
	}

	//GetCurrentFramebuffer().Bind(gl::enums::FramebufferTarget::Draw);
	//PushFramebuffer(Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH | Framebuffer::ATTACHMENT_NORMAL);
	SetCurrentFramebuffer(objectsFB);

	std::array<gl::context::BufferSelection::ColorBuffer, 3> allBuffers = {
		gl::enums::FramebufferColorAttachment::_0, gl::enums::FramebufferColorAttachment::_1, gl::enums::FramebufferColorAttachment::_2
	};

	glContext.DrawBuffers(allBuffers.size(), allBuffers.data());
	SetActiveCamera(pActiveViewerCam);
	glContext.Viewport(0, 0, screenWidth, screenHeight);
	glContext.ClearColor(0, 0, 0, 1);
	glContext.Clear().ColorBuffer().DepthBuffer();
	terrain.Draw();
	DrawObjects();

	std::array<gl::context::BufferSelection::ColorBuffer, 1> colorBuffer = {
		gl::enums::FramebufferColorAttachment::_0
	};
	glContext.DrawBuffers(colorBuffer.size(), colorBuffer.data());

	DrawAmbientOcclusion();

	skybox.Draw();

	water.Draw();

	//fog.Draw(*this);

	//(TODO) WARNING: Drawing Skybox twice! It's only purpose is to make water fade out.
	//Please note that drawing skybox ONLY after water is not a good solution, because without a skybox behind water, it will refract black background making distant water black.
	skybox.Draw();
}

void GraphicsEngine::DrawAmbientOcclusion()
{
	const int targetW = int(screenWidth * reducedSizeFramebufferScale);
	const int targetH = int(screenHeight * reducedSizeFramebufferScale);

	//TODO enable framebuffer to use a chosen number of color channels
	//so framebuffer texture reads, and writes might be optimized
	SetCurrentFramebuffer(reducedSizeIntermFramebuffer1);
	glContext.Clear().ColorBuffer();

	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_NORMAL, "objectNormal", 0);
	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_VIEW_DEPTH, "objectViewDepth", 1);
	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_DEPTH, "objectDepth", 2);
	objectsFB.SetShaderProgram(&ssaoCalcProgram, Framebuffer::ATTACHMENT_NORMAL | Framebuffer::ATTACHMENT_DEPTH | Framebuffer::ATTACHMENT_VIEW_DEPTH);
	ssaoCalcProgram.Use();

	gl::UniformSampler(ssaoCalcProgram, "noiseTex").Set(3);
	gl::Texture::Active(3);
	noise4.Bind(gl::Texture::Target::_2D);

	ssaoCalc_proj.Set(pActiveViewerCam->GetProjectionTransform());
	ssaoCalc_projInv.Set(glm::inverse(pActiveViewerCam->GetProjectionTransform()));
	IGNORE_TRY( ssaoCalc_viewTrInv.Set(glm::transpose(glm::inverse(pActiveViewerCam->GetViewTransform()))) );
	ssaoCalc_screenWidth.Set(targetW);
	ssaoCalc_screenHeight.Set(targetH);

	objectsFB.Draw(*this);

	
	//lets blur ao
	SetCurrentFramebuffer(reducedSizeIntermFramebuffer2);
	glContext.Clear().ColorBuffer();
	reducedSizeIntermFramebuffer1.SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "aoValue", 0);
	reducedSizeIntermFramebuffer1.SetShaderProgram(&ssaoBlurHorProg, Framebuffer::ATTACHMENT_COLOR);
	ssaoBlurHorProg.Use();

	ssaoBlurHor_screenWidth.Set(targetW);
	ssaoBlurHor_screenHeight.Set(targetH);
	reducedSizeIntermFramebuffer1.Draw(*this);

	SetCurrentFramebuffer(reducedSizeIntermFramebuffer1);
	glContext.Clear().ColorBuffer();
	reducedSizeIntermFramebuffer2.SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "aoValue", 0);
	reducedSizeIntermFramebuffer2.SetShaderProgram(&ssaoBlurVerProg, Framebuffer::ATTACHMENT_COLOR);
	ssaoBlurVerProg.Use();

	ssaoBlurVer_screenWidth.Set(targetW);
	ssaoBlurVer_screenHeight.Set(targetH);
	reducedSizeIntermFramebuffer2.Draw(*this);
	

	//ao is drawn to the target, lets draw it on screen
	SetCurrentFramebuffer(aoResultFB);
	glContext.Clear().ColorBuffer().DepthBuffer();

	reducedSizeIntermFramebuffer1.SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "aoValue", 2);
	reducedSizeIntermFramebuffer1.SetShaderProgram(&ssaoDrawProgram, Framebuffer::ATTACHMENT_COLOR);
	
	gl::UniformSampler(ssaoDrawProgram, "objectColor").Set(0);
	gl::Texture::Active(0);
	objectsFB.GetTexture(Framebuffer::ATTACHMENT_COLOR).Bind(gl::Texture::Target::_2D);
	gl::UniformSampler(ssaoDrawProgram, "objectDepth").Set(1);
	gl::Texture::Active(1);
	objectsFB.GetTexture(Framebuffer::ATTACHMENT_DEPTH).Bind(gl::Texture::Target::_2D);

	ssaoDraw_screenWidth.Set(screenWidth);
	ssaoDraw_screenHeight.Set(screenHeight);
	reducedSizeIntermFramebuffer1.Draw(*this);
}
