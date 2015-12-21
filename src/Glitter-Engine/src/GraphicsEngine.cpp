#include <GE/GraphicsEngine.hpp>

#include <glm/gtc/matrix_transform.hpp>

//TODO this shouldnt be included like this
#include "../../Glitter-Island/Utility.hpp"
#include <stdexcept>
#include <iostream>

std::string GraphicsEngine::shadersFolderPath = "../shaders/";
std::string GraphicsEngine::imgFolderPath = "../img/";
std::string GraphicsEngine::modelsFolderPath = "../models/";

//std::string GraphicsEngine::imgFolderPath;
//std::string GraphicsEngine::modelsFolderPath;
//std::string GraphicsEngine::shadersFolderPath;

#define IGNORE_TRY(x) try{x;}catch(std::exception&ex){std::cout<<ex.what()<<std::endl;}

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

gl::Program GraphicsEngine::LoadShaderProgramFromFiles(const std::string& vs_name, const std::string& fs_name)
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

GraphicsEngine::GraphicsEngine() :
screenWidth(0), screenHeight(0),
terrainSize(500), waterLevel(49), terrain(this), water(this, terrainSize * 7),
pActiveCam(nullptr), pActiveViewerCam(nullptr),
debugDrawer(this),
skybox(this)
{
	//terrain.LoadFromHeightMap(DemoCore::imgFolderPath + "heightMap.png", terrainSize, 0.06);
	terrain.LoadFromHeightMap(GetImgFolderPath() + "heightMap.png", terrainSize, 0.2);

	//NOTE: rotation angle does intentionally differ from exactly pi/2.
	//Reason: oglplus's matrix inversion function doesn't invert correctly for some transforms.
	terrain.SetTransform(glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(-terrainSize*0.5, -waterLevel, terrainSize*0.5)), -glm::pi<float>() / 2.f, glm::vec3(1, 0, 0)));

	skybox.LoadTextureFromFiles(
		GetImgFolderPath() + "sb4-x.bmp",
		GetImgFolderPath() + "sb1+x.bmp",
		GetImgFolderPath() + "sb6-y.bmp",
		GetImgFolderPath() + "sb3+y.bmp",
		GetImgFolderPath() + "sb2-z.bmp",
		GetImgFolderPath() + "sb5+z.bmp");

	sun.SetColor(glm::vec3(1, 1, 1));
	
	intermediateFramebuffer = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	aoResultFB = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	objectsFB = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH | Framebuffer::ATTACHMENT_NORMAL);
	halfSizedIntermFramebuffer = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	AddFramebufferForManagment(intermediateFramebuffer);
	AddFramebufferForManagment(aoResultFB);
	AddFramebufferForManagment(objectsFB);

	finalFramebufferCopy = LoadShaderProgramFromFiles("FinalFramebufferCopy_v.glsl", "FinalFramebufferCopy_f.glsl");
	finalFramebufferCopy.Use();
	framebufferCopy_ScreenWidth = gl::Uniform<GLint>(finalFramebufferCopy, "screenWidth");
	framebufferCopy_ScreenHeight = gl::Uniform<GLint>(finalFramebufferCopy, "screenHeight");
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	ssaoCalcProgram = LoadShaderProgramFromFiles("Passthrough2_v.glsl", "SSAO_Calc_f.glsl");
	ssaoCalcProgram.Use();
	IGNORE_TRY(ssaoCalc_viewProj = gl::Uniform<glm::mat4>(ssaoCalcProgram, "viewProj"));
	IGNORE_TRY(ssaoCalc_viewProjInv = gl::Uniform<glm::mat4>(ssaoCalcProgram, "viewProjInv"));
	IGNORE_TRY(ssaoCalc_screenWidth = gl::Uniform<GLint>(ssaoCalcProgram, "screenWidth"));
	IGNORE_TRY(ssaoCalc_screenHeight = gl::Uniform<GLint>(ssaoCalcProgram, "screenHeight"));


	ssaoDrawProgram = LoadShaderProgramFromFiles("Passthrough2_v.glsl", "SSAO_Draw_f.glsl");
	ssaoDrawProgram.Use();
	//IGNORE_TRY(ssaoCalc_viewProj = gl::Uniform<glm::mat4>(ssaoDrawProgram, "viewProj"));
	//IGNORE_TRY(ssaoCalc_viewProjInv = gl::Uniform<glm::mat4>(ssaoDrawProgram, "viewProjInv"));
	IGNORE_TRY(ssaoDraw_screenWidth = gl::Uniform<GLint>(ssaoDrawProgram, "screenWidth"));
	IGNORE_TRY(ssaoDraw_screenHeight = gl::Uniform<GLint>(ssaoDrawProgram, "screenHeight"));

	debugDrawer.SetEnabled(false);
	
	wireframeModeEnabled = false;
	
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
	intermediateFramebuffer.Bind(gl::Framebuffer::Target::Draw);
	glContext.Disable(gl::Capability::Blend);
	glContext.Enable(gl::Capability::DepthTest);
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

	for (auto current : managedFramebuffers) {
		current->SetResolution(width, height);
	}

	halfSizedIntermFramebuffer.SetResolution(width/2, height/2);
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
	managedFramebuffers.push_back(&framebuffer);
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

void GraphicsEngine::SetActiveCamera(Camera* cam)
{
	pActiveCam = cam;
}

Camera* GraphicsEngine::GetActiveCamera()
{
	return pActiveCam;
}

void GraphicsEngine::SetActiveViewerCamera(PerspectiveCamera* cam)
{
	pActiveViewerCam = cam;
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

void GraphicsEngine::AddGraphicalObject(GraphicalObject&& newObject)
{
	managedGraphicalObjects.push_back(std::move(newObject));
	GraphicalObject& moved = managedGraphicalObjects.back();

	for (auto& currSubmesh : moved.GetMesh()->GetSubmeshes()) {
		instancedGraphicalObjects[&currSubmesh].push_back(&moved);
		std::cout << "count: " << instancedGraphicalObjects[&currSubmesh].size() << std::endl;
	}
}

void GraphicsEngine::AddGraphicalObject(GraphicalObject* newObject)
{
	externalGraphicalObjects.push_back(newObject);
}

Mesh* GraphicsEngine::LoadMeshFromFile(const std::string& filename)
{
	return meshManager.LoadMeshFromFile(this, GraphicsEngine::modelsFolderPath + filename);
}

Material* GraphicsEngine::LoadStandardMaterialFromFile(const std::string& filename, const std::string& materialName)
{
	return materialManager.LoadStandardMaterialFromFile(this, GraphicsEngine::modelsFolderPath + filename, materialName);
}

GraphicalObject GraphicsEngine::LoadGraphicalObjectFromFile(const std::string& filename)
{
	GraphicalObject result;

	{
		Mesh* pMesh = LoadMeshFromFile(filename);

		result.SetMesh(pMesh);
	}

	return std::move(result);
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
	lightCascadeShadowMapFramebuffers[cascadeID].Bind(gl::enums::FramebufferTarget::Draw);
	//Draw depth only!
	glContext.DrawBuffer(gl::enums::ColorBuffer::None);

	SetActiveCamera(&(lightCascadeCameras[cascadeID]));
	glContext.Viewport(0, 0, shadowMapResX, shadowMapResY);
	glContext.Clear().DepthBuffer();
	glContext.CullFace(gl::enums::Face::Front);
	for (auto& current : managedGraphicalObjects) {
		current.Draw(this);
	}
	for (auto current : externalGraphicalObjects) {
		//current->Draw(this);
	}
	glContext.CullFace(gl::enums::Face::Back);
}

void GraphicsEngine::DrawObjects()
{
	terrain.Draw();

	//for (auto& current : managedGraphicalObjects) {
	//	current.Draw(this);
	//}

	glContext.Disable(gl::Capability::CullFace);
	//glContext.Enable(gl::Capability::Blend);
	//glContext.BlendFunc(gl::enums::BlendFunction::SrcAlpha, gl::enums::BlendFunction::OneMinusSrcAlpha);
	glContext.Disable(gl::Capability::Blend);

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

	glContext.Disable(gl::Capability::Blend);
	glContext.Enable(gl::Capability::CullFace);

	for (auto current : externalGraphicalObjects) {
		current->Draw(this);
	}
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
	std::vector<gl::context::BufferSelection::ColorBuffer> selectedBuffers = {
		gl::enums::FramebufferColorAttachment::_0, gl::enums::FramebufferColorAttachment::_1
	};
	glContext.DrawBuffers(selectedBuffers);
	SetActiveCamera(pActiveViewerCam);
	glContext.Viewport(0, 0, screenWidth, screenHeight);
	glContext.ClearColor(0, 0, 0, 1);
	glContext.Clear().ColorBuffer().DepthBuffer();
	DrawObjects();

	selectedBuffers.pop_back();
	glContext.DrawBuffers(selectedBuffers);

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
	const int targetW = screenWidth/2;
	const int targetH = screenHeight/2;

	//TODO enable framebuffer to use a chosen number of color channels
	//so framebuffer texture reads, and writes might be optimized
	SetCurrentFramebuffer(halfSizedIntermFramebuffer);
	glContext.Clear().ColorBuffer();

	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_NORMAL, "objectNormal", 0);
	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_DEPTH, "objectDepth", 1);
	objectsFB.SetShaderProgram(&ssaoCalcProgram, Framebuffer::ATTACHMENT_NORMAL | Framebuffer::ATTACHMENT_DEPTH);
	ssaoCalcProgram.Use();

	ssaoCalc_viewProj.Set(pActiveViewerCam->GetViewProjectionTransform());
	ssaoCalc_viewProjInv.Set(glm::inverse(pActiveViewerCam->GetViewProjectionTransform()));
	IGNORE_TRY( ssaoCalc_screenWidth.Set(targetW) );
	IGNORE_TRY( ssaoCalc_screenHeight.Set(targetH) );

	objectsFB.Draw(*this);
	

	//ao is drawn to the target, lets draw it on screen
	SetCurrentFramebuffer(aoResultFB);
	glContext.Clear().ColorBuffer().DepthBuffer();

	halfSizedIntermFramebuffer.SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "aoValue", 2);
	halfSizedIntermFramebuffer.SetShaderProgram(&ssaoDrawProgram, Framebuffer::ATTACHMENT_COLOR);
	
	gl::UniformSampler(ssaoDrawProgram, "objectColor").Set(0);
	gl::Texture::Active(0);
	objectsFB.GetTexture(Framebuffer::ATTACHMENT_COLOR).Bind(gl::Texture::Target::_2D);
	gl::UniformSampler(ssaoDrawProgram, "objectDepth").Set(1);
	gl::Texture::Active(1);
	objectsFB.GetTexture(Framebuffer::ATTACHMENT_DEPTH).Bind(gl::Texture::Target::_2D);

	IGNORE_TRY( ssaoDraw_screenWidth.Set(screenWidth) );
	IGNORE_TRY( ssaoDraw_screenHeight.Set(screenHeight) );
	halfSizedIntermFramebuffer.Draw(*this);
}
