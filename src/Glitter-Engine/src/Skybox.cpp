#include <GE/Skybox.hpp>

#include <SFML/Graphics.hpp>
#include <GE/GraphicsEngine.hpp>

Skybox::Skybox(GraphicsEngine* pGraphicsEngine)
: pGraphEngine(pGraphicsEngine)
{
	skydrawShader = GraphicsEngine::LoadShaderProgramFromFiles("Skybox_skydraw_v.glsl", "Skybox_skydraw_f.glsl");
	skydrawShader.Use();

	sh_ViewProjectionMatrix = gl::Uniform<gl::Mat4f>(skydrawShader, "viewProjectionMatrix");

	VAO.Bind();

	std::vector<GLfloat> vertexPosData = {
		-1.0f,-1.0f,-1.0f,
		+1.0f,-1.0f,-1.0f,
		-1.0f,+1.0f,-1.0f,
		+1.0f,+1.0f,-1.0f,
		-1.0f,-1.0f,+1.0f,
		+1.0f,-1.0f,+1.0f,
		-1.0f,+1.0f,+1.0f,
		+1.0f,+1.0f,+1.0f
	};

	vertexPositions.Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, vertexPosData);
	gl::VertexArrayAttrib vert_attr(skydrawShader, "vertexPos");
	vert_attr.Setup<gl::Vec3f>().Enable();

	std::vector<GLushort> indexData = {
		1, 3, 5, 7, 9,
		4, 6, 0, 2, 9,
		2, 6, 3, 7, 9,
		4, 0, 5, 1, 9,
		5, 7, 4, 6, 9,
		0, 2, 1, 3, 9
	};

	//At Drawnig...
	//gl.Enable(Capability::PrimitiveRestart);
	//gl.PrimitiveRestartIndex(9);

	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexData);

	///Fadeout shader
	fadeoutShader = GraphicsEngine::LoadShaderProgramFromFiles("Passthrough2_v.glsl", "Skybox_fadeout_f.glsl");
	fadeoutShader.Use();

	//gl::UniformSampler(fadeoutShader, "screenColor").Set(0);
	//gl::UniformSampler(fadeoutShader, "screenDepth").Set(1);
	gl::UniformSampler(fadeoutShader, "skyboxColor").Set(2);
	
	resultFB = Framebuffer(0, 0, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);
	pGraphEngine->AddFramebufferForManagment(resultFB);
}

void Skybox::LoadTextureFromFiles(
	const std::string& negXfileName,
	const std::string& posXfileName,
	const std::string& negYfileName,
	const std::string& posYfileName,
	const std::string& negZfileName,
	const std::string& posZfileName)
{
	VAO.Bind();
	skydrawShader.Use();

	gl::UniformSampler(skydrawShader, "cubeMap").Set(0);
	gl::Texture::Active(0);
	cubeMap.Bind(gl::Texture::Target::CubeMap);

	gl::Texture::MinFilter(gl::Texture::Target::CubeMap, gl::TextureMinFilter::Linear);
	gl::Texture::MagFilter(gl::Texture::Target::CubeMap, gl::TextureMagFilter::Linear);

	gl::Texture::WrapS(gl::Texture::Target::CubeMap, gl::TextureWrap::ClampToEdge);
	gl::Texture::WrapT(gl::Texture::Target::CubeMap, gl::TextureWrap::ClampToEdge);
	gl::Texture::WrapR(gl::Texture::Target::CubeMap, gl::TextureWrap::ClampToEdge);


	std::map<gl::Texture::Target, std::string> imageContainer = {
		{gl::Texture::Target::CubeMapNegativeX, negXfileName},
		{gl::Texture::Target::CubeMapPositiveX, posXfileName},
		{gl::Texture::Target::CubeMapNegativeY, negYfileName},
		{gl::Texture::Target::CubeMapPositiveY, posYfileName},
		{gl::Texture::Target::CubeMapNegativeZ, negZfileName},
		{gl::Texture::Target::CubeMapPositiveZ, posZfileName}
	};

	for (auto& current : imageContainer) {
		gl::Texture::Target bindTarget = current.first;

		std::string& fileName = current.second;
		sf::Image image;

		if (!image.loadFromFile(fileName)) {
			throw std::runtime_error(std::string("Can not load image file: ") + fileName);
		}

		gl::Texture::Image2D(
			bindTarget,
			0,
			gl::enums::PixelDataInternalFormat::SRGB8,
			image.getSize().x,
			image.getSize().y,
			0,
			gl::PixelDataFormat::RGBA,
			//gl::Texture::Property::PixDataType::OneOf(gl::DataType::UnsignedByte),
			gl::DataType::UnsignedByte,
			image.getPixelsPtr());
	}
}

void Skybox::Draw()
{
	gl::Context& glContext = pGraphEngine->GetGLContext();

	auto& screenFB = pGraphEngine->GetCurrentFramebuffer();
	auto& intermediateFB = pGraphEngine->GetIntermediateFramebuffer();
	pGraphEngine->SetCurrentFramebuffer(intermediateFB);
	glContext.Clear().ColorBuffer().DepthBuffer();

	skydrawShader.Use();

	gl::Texture::Active(0);
	cubeMap.Bind(gl::Texture::Target::CubeMap);

	gl::Mat4f viewOnlyRotation = pGraphEngine->GetActiveCamera()->GetViewTransform();
	viewOnlyRotation.Set(0, 3, 0);
	viewOnlyRotation.Set(1, 3, 0);
	viewOnlyRotation.Set(2, 3, 0);
	viewOnlyRotation.Set(3, 3, 1);
	viewOnlyRotation.Set(3, 0, 0);
	viewOnlyRotation.Set(3, 1, 0);
	viewOnlyRotation.Set(3, 2, 0);
	gl::Mat4f projectionMatrix = pGraphEngine->GetActiveCamera()->GetProjectionTransform();

	sh_ViewProjectionMatrix.Set(projectionMatrix * viewOnlyRotation);

	VAO.Bind();

	glContext.Disable(gl::Capability::CullFace);
	glContext.Enable(gl::Capability::PrimitiveRestart);
	glContext.PrimitiveRestartIndex(9);
	//glContext.Enable(gl::Capability::DepthTest);
	glContext.DrawElements(gl::PrimitiveType::TriangleStrip, 6*5, gl::DataType::UnsignedShort);
	glContext.Disable(gl::Capability::PrimitiveRestart);
	glContext.Enable(gl::Capability::CullFace);

	//Draw fadeout
	pGraphEngine->SetCurrentFramebuffer(resultFB);
	glContext.Clear().DepthBuffer();

	screenFB.SetTextureShaderID(Framebuffer::ATTACHMENT_COLOR, "screenColor", 0);
	screenFB.SetTextureShaderID(Framebuffer::ATTACHMENT_DEPTH, "screenDepth", 1);
	screenFB.SetShaderProgram(&fadeoutShader, Framebuffer::ATTACHMENT_COLOR | Framebuffer::ATTACHMENT_DEPTH);

	fadeoutShader.Use();

	gl::Texture::Active(2);
	intermediateFB.GetTexture(Framebuffer::ATTACHMENT_COLOR).Bind(gl::Texture::Target::_2D);

	//Problem is the fadeout shader writes 1 where the skybox is. So its color will be ignored due to depth testing at .
	screenFB.Draw(*pGraphEngine);
}
