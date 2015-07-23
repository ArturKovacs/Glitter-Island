#include "Water.hpp"

#include "DemoCore.hpp"
#include "FileLoad.hpp"
#include <cfloat>

Water::Water(const float waterSize)
{
	waterShader = DemoCore::LoadShaderProgramFromFiles("water_v.glsl", "water_f.glsl");
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
	auto& gl = core.GetGLContext();

	auto& screenFB = core.GetCurrentFramebuffer();
	core.PushFramebuffer();
	core.CopyFramebufferContents(screenFB);

	waterShader.Use();

	sh_screen.Set(0);
	gl::Texture::Active(0);
	screenFB.GetColorTexture().Bind(gl::Texture::Target::_2D);

	sh_screenDepth.Set(1);
	gl::Texture::Active(1);
	screenFB.GetDepthTexture().Bind(gl::Texture::Target::_2D);

	sh_screenWidth.Set(core.GetCamera().GetScreenWidth());
	sh_screenHeight.Set(core.GetCamera().GetScreenHeight());

	sh_MVP.Set(core.GetCamera().GetViewProjectionTransform());
	sh_viewProj.Set(core.GetCamera().GetViewProjectionTransform());
	//sh_invMVP.Set(gl::Inverse(core.GetCamera().GetViewProjectionTransform()));
	sh_camPos.Set(core.GetCamera().GetPosition());
	sh_sunDir.Set(core.GetSun().GetDirectionTowardsSource());
	sh_time.Set(core.GetElapsedTime().asSeconds());

	VAO.Bind();

	gl.Enable(gl::Capability::Blend);
	gl.BlendFunc(gl::enums::BlendFunction::SrcAlpha, gl::enums::BlendFunction::OneMinusSrcAlpha);

	gl.DrawElements(gl::PrimitiveType::TriangleFan, indices.Size(gl::Buffer::Target::ElementArray).get() / sizeof(GLushort), gl::DataType::UnsignedShort);

	gl.Disable(gl::Capability::Blend);
}
