#include <GE/GraphicsEngine.hpp>

//TODO this shouldnt be included like this
#include "../../Glitter-Island/Utility.hpp"
#include <stdexcept>

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

#define IGNORE_TRY(x) try{x;}catch(std::exception&ex){std::cout<<ex.what()<<std::endl;}

GraphicsEngine::GraphicsEngine() :
screenWidth(0), screenHeight(0),
terrainSize(500), waterLevel(49), terrain(this), water(terrainSize * 7),
pActiveCam(nullptr), pActiveViewerCam(nullptr),
debugDrawer(this)
{
	//terrain.LoadFromHeightMap(DemoCore::imgFolderPath + "heightMap.png", terrainSize, 0.06);
	terrain.LoadFromHeightMap(GetImgFolderPath() + "heightMap.png", terrainSize, 0.2);

	//NOTE: rotation angle does intentionally differ from exactly pi/2.
	//Reason: oglplus's matrix inversion function doesn't invert correctly for some transforms.
	terrain.SetTransform(gl::ModelMatrixf::Translation(-terrainSize*0.5, -waterLevel, terrainSize*0.5)*gl::ModelMatrixf::RotationA(gl::Vec3f(1, 0, 0), gl::Radians<float>(-gl::math::Pi() / 2.001)));

	skybox.LoadTextureFromFiles(
		GetImgFolderPath() + "sb4-x.bmp",
		GetImgFolderPath() + "sb1+x.bmp",
		GetImgFolderPath() + "sb6-y.bmp",
		GetImgFolderPath() + "sb3+y.bmp",
		GetImgFolderPath() + "sb2-z.bmp",
		GetImgFolderPath() + "sb5+z.bmp");

	sun.SetColor(gl::Vec3f(1, 1, 1));
	
	PushFramebuffer();

	finalFramebufferCopy = LoadShaderProgramFromFiles("FinalFramebufferCopy_v.glsl", "FinalFramebufferCopy_f.glsl");
	finalFramebufferCopy.Use();
	framebufferCopy_ScreenWidth = gl::Uniform<GLint>(finalFramebufferCopy, "screenWidth");
	framebufferCopy_ScreenHeight = gl::Uniform<GLint>(finalFramebufferCopy, "screenHeight");
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);
	//gl::UniformSampler(finalFramebufferCopy, "colorTex").Set(0);
	//gl::UniformSampler(finalFramebufferCopy, "depthTex").Set(1);

	ssaoProgram = LoadShaderProgramFromFiles("Passthrough2_v.glsl", "SSAO_f.glsl");
	ssaoProgram.Use();
	IGNORE_TRY(ssao_viewProj = gl::Uniform<gl::Mat4f>(ssaoProgram, "viewProj"));
	IGNORE_TRY(ssao_viewProjInv = gl::Uniform<gl::Mat4f>(ssaoProgram, "viewProjInv"));
	gl::Uniform<gl::Mat4f> ssao_viewProj;
	gl::Uniform<gl::Mat4f> ssao_viewProjInv;

	IGNORE_TRY(ssao_screenWidth = gl::Uniform<GLint>(ssaoProgram, "screenWidth"));
	IGNORE_TRY(ssao_screenHeight = gl::Uniform<GLint>(ssaoProgram, "screenHeight"));

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
		currFramebuffer = std::move(Framebuffer{shadowMapResolution, shadowMapResolution, Framebuffer::ATTACHMENT_DEPTH});
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
	ClearFramebufferStack();
	GetCurrentFramebuffer().Bind(gl::Framebuffer::Target::Draw);
	glContext.Disable(gl::Capability::Blend);
	glContext.Enable(gl::Capability::DepthTest);
	//pContextManager->Draw();
	DrawScene();

	//draw current framebuffer to screen
	GetCurrentFramebuffer().SetVertexPosName("vertexPos");
	GetCurrentFramebuffer().SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "colorTex", 0);
	GetCurrentFramebuffer().SetTextureShaderID(Framebuffer::ATTACHMENT_DEPTH, "depthTex", 1);
	GetCurrentFramebuffer().SetShaderProgram(&finalFramebufferCopy);

	defaultFBO.Bind(gl::Framebuffer::Target::Draw);
	glContext.Clear().ColorBuffer().DepthBuffer();
	GetCurrentFramebuffer().Draw(*this, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);

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

	//textDrawer.SetScreenResolution(gl::Vec2i(screenWidth, screenHeight));
	
	finalFramebufferCopy.Use();
	framebufferCopy_ScreenWidth.Set(screenWidth);
	framebufferCopy_ScreenHeight.Set(screenHeight);

	for (auto& current : framebuffers) {
		current.SetResolution(width, height);
	}
}

int GraphicsEngine::GetScreenWidth() const
{
	return screenWidth;
}

int GraphicsEngine::GetScreenHeight() const
{
	return screenHeight;
}

void GraphicsEngine::PushFramebuffer(uint8_t framebufferAttachmentFlags)
{
	framebuffers.push_back(std::move(Framebuffer(screenWidth, screenHeight, framebufferAttachmentFlags)));
	framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
}

void GraphicsEngine::PopFramebuffer()
{
	if (framebuffers.size() > 1) {
		framebuffers.pop_back();
		framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
	}
}

void GraphicsEngine::CopyFramebufferContents(const Framebuffer& source)
{
	source.Bind(gl::Framebuffer::Target::Read);
	glContext.BlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, gl::Bitfield<gl::enums::BufferSelectBit>(gl::enums::BufferSelectBit::DepthBuffer) |= gl::enums::BufferSelectBit::ColorBuffer, gl::enums::BlitFilter::Nearest);
}

Framebuffer& GraphicsEngine::GetCurrentFramebuffer()
{
	return framebuffers.back();
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

gl::Mat4f GraphicsEngine::GetCascadeViewProjectionTransform(int cascadeID) const
{
	return lightCascadeCameras.at(cascadeID).GetViewProjectionTransform();
}

float GraphicsEngine::GetViewSubfrustumFarPlaneInTexCoordZ(int subfrustumID) const
{
	const float totalDepth = pActiveViewerCam->GetZFar() - pActiveViewerCam->GetZNear();
	gl::Vec4f farPlanePosInViewSpace = gl::Vec4f(0, 0, -1*(pActiveViewerCam->GetZNear() + totalDepth*subfrustumFarPlanePositionRatios.at(subfrustumID)), 1);
	gl::Vec4f transformResult = pActiveViewerCam->GetProjectionTransform() * farPlanePosInViewSpace;
	return (transformResult.z() / transformResult.w())*0.5f + 0.5f;
}

void GraphicsEngine::AddGraphicalObject(GraphicalObject&& newObject)
{
	managedGraphicalObjects.push_back(std::move(newObject));
	GraphicalObject& moved = managedGraphicalObjects.back();

	for (auto& currSubmesh : moved.GetMesh()->GetSubmeshes()) {
		instancedGraphicalObjects[&currSubmesh].push_back(&moved);
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

void GraphicsEngine::ClearFramebufferStack()
{
	framebuffers.resize(1);

	framebuffers.back().Bind(gl::Framebuffer::Target::Draw);
}

void GraphicsEngine::UpdateLightViewTransform()
{
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
	/// Use transpose instead of expensive inverse since this matrix is orthogonal.
	/// The elements are written down transposed so this matrix
	/// is already transposed when constructed.
	lightViewTransform = gl::Mat4f(
		lightViewX.x(), lightViewX.y(), lightViewX.z(), 0,
		lightViewY.x(), lightViewY.y(), lightViewY.z(), 0,
		lightViewZ.x(), lightViewZ.y(), lightViewZ.z(), 0,
		             0,              0,              0, 1);
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

	{
		using planeType = decltype(subFrustum.nearPlane);
		static_assert(std::is_same<gl::Vec3f, planeType::value_type>::value, "Error plane quad does not consist of expected types.");
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
	glContext.Viewport(0, 0, shadowMapResolution, shadowMapResolution);
	glContext.Clear().DepthBuffer();

	for (auto& current : managedGraphicalObjects) {
		current.Draw(this);
	}
	for (auto current : externalGraphicalObjects) {
		//current->Draw(this);
	}
}

void GraphicsEngine::DrawObjects()
{
	terrain.Draw();

	//for (auto& current : managedGraphicalObjects) {
	//	current.Draw(this);
	//}

	glContext.Disable(gl::Capability::CullFace);
	glContext.Enable(gl::Capability::Blend);
	gl::Context::BlendFunc(gl::enums::BlendFunction::SrcAlpha, gl::enums::BlendFunction::OneMinusSrcAlpha);

	for (auto& current : instancedGraphicalObjects) {
		Mesh::Submesh* submesh = current.first;
		Material* pMaterial = submesh->GetMaterial();
		if (pMaterial != nullptr) {
			submesh->BindVAO();
			auto& vector = current.second;
			pMaterial->Prepare(*submesh);
			for(auto currentObject : vector) {
				if (currentObject->IsVisible()){
					if (currentObject->IsDepthTestEnabled()) {
						glContext.Enable(gl::Capability::DepthTest);
					}
					else {
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

	GetCurrentFramebuffer().Bind(gl::enums::FramebufferTarget::Draw);
	PushFramebuffer(Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH | Framebuffer::ATTACHMENT_NORMAL);
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

	DrawAmbientOcclusion(GetCurrentFramebuffer());

	skybox.Draw(*this);

	water.Draw(*this);

//	fog.Draw(*this);

	//(TODO) WARNING: Drawing Skybox twice! It's only purpose is to make water fade out.
	//Please note that drawing skybox ONLY after water is not a good solution, because without a skybox behind water, it will refract black background making distant water black.
	skybox.Draw(*this);
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

void GraphicsEngine::DrawAmbientOcclusion(Framebuffer& objectsFB)
{
	PushFramebuffer(Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	glContext.Clear().ColorBuffer().DepthBuffer();

	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "objectColor", 0);
	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_NORMAL, "objectNormal", 1);
	objectsFB.SetTextureShaderID(Framebuffer::ATTACHMENT_DEPTH, "objectDepth", 2);
	objectsFB.SetShaderProgram(&ssaoProgram);
	ssaoProgram.Use();
	//IGNORE_TRY( ssao_view_TrInv.Set(MyTranspose(gl::Inverse(pActiveViewerCam->GetViewTransform()))) );
	//IGNORE_TRY( ssao_projInv.Set(gl::Inverse(pActiveViewerCam->GetProjectionTransform())) );
	//IGNORE_TRY( ssao_proj.Set(pActiveViewerCam->GetProjectionTransform()) );

	ssao_viewProj.Set(pActiveViewerCam->GetViewProjectionTransform());
	ssao_viewProjInv.Set(gl::Inverse(pActiveViewerCam->GetViewProjectionTransform()));

	//IGNORE_TRY( ssao_projInv_TrInv.Set(MyTranspose(pActiveViewerCam->GetProjectionTransform())) );
	IGNORE_TRY( ssao_screenWidth.Set(screenWidth) );
	IGNORE_TRY( ssao_screenHeight.Set(screenHeight) );

	objectsFB.Draw(*this, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_NORMAL | Framebuffer::ATTACHMENT_DEPTH);
	//objectsFB.Draw(*this, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	//objectsFB.Draw(*this, Framebuffer::ATTACHMENT_NORMAL);
}
