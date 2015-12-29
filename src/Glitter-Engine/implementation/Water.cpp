#include <GE/Water.hpp>

#include <GE/GraphicsEngine.hpp>

Water::Water(GraphicsEngine* pGraphicsEngine, const float waterSize) :
pGraphEngine(pGraphicsEngine), visible(true)
{
	waterShader = GraphicsEngine::LoadShaderProgramFromFiles("Water_v.glsl", "Water_f.glsl");
	waterShader.Use();

	sh_MVP = gl::Uniform<glm::mat4>(waterShader, "MVP");
	sh_invViewProj = gl::Uniform<glm::mat4>(waterShader, "invViewProj");
	//sh_invMVP = gl::Uniform<glm::mat4>(waterShader, "invMVP");
	sh_screen = gl::UniformSampler(waterShader, "screen");
	sh_screenDepth = gl::UniformSampler(waterShader, "screenDepth");
	sh_screenWidth = gl::Uniform<GLint>(waterShader, "screenWidth");
	sh_screenHeight = gl::Uniform<GLint>(waterShader, "screenHeight");
	sh_camPos = gl::Uniform<glm::vec3>(waterShader, "campos");
	sh_sunDir = gl::Uniform<glm::vec3>(waterShader, "sunDir");
	sh_sunColor = gl::Uniform<glm::vec3>(waterShader, "sunColor");
	sh_time = gl::Uniform<GLfloat>(waterShader, "time");

	VAO.Bind();

	const float waterHeight = 0.0;

	std::vector<GLfloat> vertexPosData = {
		-waterSize, waterHeight, +waterSize,
		+waterSize, waterHeight, +waterSize,
		+waterSize, waterHeight, -waterSize,
		-waterSize, waterHeight, -waterSize
	};

	vertexPos.Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, vertexPosData);
	gl::VertexArrayAttrib vertAttr(waterShader, "vertexPos");
	vertAttr.Setup(3, gl::DataType::Float).Enable();

	std::vector<GLushort> indexData = {
		0, 1, 2, 3
	};

	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexData);
	
	targetFB = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	pGraphEngine->AddFramebufferForManagment(targetFB);
}

void Water::Draw()
{
	if(!visible) {
		return;
	}
		
	auto& gl = pGraphEngine->GetGLContext();

	auto& screenFB = pGraphEngine->GetCurrentFramebuffer();
	pGraphEngine->SetCurrentFramebuffer(targetFB);
	targetFB.CopyFramebufferContents(screenFB);

	waterShader.Use();

	sh_screen.Set(0);
	gl::Texture::Active(0);
	screenFB.GetTexture(Framebuffer::ATTACHMENT_COLOR).Bind(gl::Texture::Target::_2D);

	sh_screenDepth.Set(1);
	gl::Texture::Active(1);
	screenFB.GetTexture(Framebuffer::ATTACHMENT_DEPTH).Bind(gl::Texture::Target::_2D);

	sh_screenWidth.Set(pGraphEngine->GetScreenWidth());
	sh_screenHeight.Set(pGraphEngine->GetScreenHeight());

	sh_MVP.Set(pGraphEngine->GetActiveViewerCamera()->GetViewProjectionTransform());
	sh_invViewProj.Set(glm::inverse(pGraphEngine->GetActiveViewerCamera()->GetViewProjectionTransform()));
	//sh_invMVP.Set(gl::Inverse(core.GetCamera().GetViewProjectionTransform()));

	PerspectiveCamera* pActivePerspectiveCam = dynamic_cast<PerspectiveCamera*>(pGraphEngine->GetActiveViewerCamera());
	if (pActivePerspectiveCam == nullptr) {
		throw std::runtime_error("Active camera was not a PerspectiveCamera when rendering water.");
	}

	sh_camPos.Set(pActivePerspectiveCam->GetPosition());
	sh_sunDir.Set(pGraphEngine->GetSun().GetDirectionTowardsSource());
	sh_sunColor.Set(pGraphEngine->GetSun().GetColor());
	sh_time.Set(static_cast<float>(pGraphEngine->GetElapsedSeconds()));

	VAO.Bind();

	gl.Disable(gl::Capability::CullFace);
	gl.Disable(gl::Capability::Blend);
	gl.DrawElements(gl::PrimitiveType::TriangleFan, indices.Size(gl::Buffer::Target::ElementArray).get() / sizeof(GLushort), gl::DataType::UnsignedShort);
}
