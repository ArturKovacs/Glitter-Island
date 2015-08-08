#include "Fog.hpp"

#include "DemoCore.hpp"
#include "Utility.hpp"

Fog::Fog()
{
	gl::VertexShader vs;
	vs.Source(util::LoadFileAsString(DemoCore::shadersFolderPath + "Fog_v.glsl"));
	vs.Compile();
	shaderProgram.AttachShader(vs);

	gl::FragmentShader fs;
	fs.Source(util::LoadFileAsString(DemoCore::shadersFolderPath + "Fog_f.glsl"));
	fs.Compile();
	shaderProgram.AttachShader(fs);

	shaderProgram.Link();
	shaderProgram.Use();

	//gl::Texture::Active(0);
	gl::UniformSampler(shaderProgram, "screenColor").Set(0);

	//gl::Texture::Active(1);
	gl::UniformSampler(shaderProgram, "screenDepth").Set(1);
}

void Fog::Draw(DemoCore& core)
{
	auto& screenFB = core.GetCurrentFramebuffer();
	core.PushFramebuffer();

	screenFB.SetColorTexName("screenColor");
	screenFB.SetDepthTexName("screenDepth");
	screenFB.SetVertexPosName("vertexPos");
	screenFB.SetShaderProgram(&shaderProgram);

	core.GetGLContext().Disable(gl::Capability::DepthTest);

	screenFB.Draw(core);
}
