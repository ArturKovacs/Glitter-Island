#include "Water.hpp"

#include "DemoCore.hpp"

Water::Water(const float waterSize)
{
	waterShader = GraphicsEngine::LoadShaderProgramFromFiles("Water_v.glsl", "Water_f.glsl");
	waterShader.Use();

	sh_MVP = gl::Uniform<gl::Mat4f>(waterShader, "MVP");
	sh_viewProj = gl::Uniform<gl::Mat4f>(waterShader, "viewProj");
	//sh_invMVP = gl::Uniform<gl::Mat4f>(waterShader, "invMVP");
	sh_screen = gl::UniformSampler(waterShader, "screen");
	sh_screenDepth = gl::UniformSampler(waterShader, "screenDepth");
	sh_screenWidth = gl::Uniform<GLint>(waterShader, "screenWidth");
	sh_screenHeight = gl::Uniform<GLint>(waterShader, "screenHeight");
	sh_camPos = gl::Uniform<gl::Vec3f>(waterShader, "campos");
	sh_sunDir = gl::Uniform<gl::Vec3f>(waterShader, "sunDir");
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
	gl::VertexArrayAttrib vert_attr(waterShader, "vertexPos");
	vert_attr.Setup<gl::Vec3f>().Enable();

	std::vector<GLushort> indexData = {
		0, 1, 2, 3
	};

	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexData);
}

void Water::Draw(DemoCore& core)
{
	auto& gl = core.GetGraphicsEngine().GetGLContext();

	auto& screenFB = core.GetGraphicsEngine().GetCurrentFramebuffer();
	core.GetGraphicsEngine().PushFramebuffer();
	core.GetGraphicsEngine().CopyFramebufferContents(screenFB);

	waterShader.Use();

	sh_screen.Set(0);
	gl::Texture::Active(0);
	screenFB.GetColorTexture().Bind(gl::Texture::Target::_2D);

	sh_screenDepth.Set(1);
	gl::Texture::Active(1);
	screenFB.GetDepthTexture().Bind(gl::Texture::Target::_2D);

	sh_screenWidth.Set(core.GetScreenWidth());
	sh_screenHeight.Set(core.GetScreenHeight());

	sh_MVP.Set(core.GetGraphicsEngine().GetActiveCamera()->GetViewProjectionTransform());
	sh_viewProj.Set(core.GetGraphicsEngine().GetActiveCamera()->GetViewProjectionTransform());
	//sh_invMVP.Set(gl::Inverse(core.GetCamera().GetViewProjectionTransform()));

	PerspectiveCamera* pActivePerspectiveCam = dynamic_cast<PerspectiveCamera*>(core.GetGraphicsEngine().GetActiveCamera());
	if (pActivePerspectiveCam == nullptr) {
		throw std::runtime_error("Active camera was not a PerspectiveCamera when rendering water.");
	}

	sh_camPos.Set(pActivePerspectiveCam->GetPosition());
	sh_sunDir.Set(core.GetGraphicsEngine().GetSun().GetDirectionTowardsSource());
	sh_time.Set(core.GetElapsedTime().asSeconds());

	VAO.Bind();

	gl.Disable(gl::Capability::CullFace);
	gl.Disable(gl::Capability::Blend);
	gl.DrawElements(gl::PrimitiveType::TriangleFan, indices.Size(gl::Buffer::Target::ElementArray).get() / sizeof(GLushort), gl::DataType::UnsignedShort);
}
