#include <GE/StandardMaterial.hpp>

#include <GE/GraphicsEngine.hpp>

StandardMaterial::StandardMaterial(GraphicsEngine* pGraphicsEngine) : pGraphicsEngine(pGraphicsEngine)
{
	//TODO shader loading might be better to be done by DemoCore too.
	shaderProgram = GraphicsEngine::LoadShaderProgramFromFiles("StandardMaterial_v.glsl", "StandardMaterial_f.glsl");
	shaderProgram.Use();

	try {
		sh_MVP = gl::Uniform<gl::Mat4f>(shaderProgram, "MVP");
		sh_MODELVIEW = gl::Uniform<gl::Mat4f>(shaderProgram, "MODELVIEW");
		sh_modelTransposedInverse = gl::Uniform<gl::Mat4f>(shaderProgram, "model_transposed_inverse");
		sh_lightDir = gl::Uniform<gl::Vec3f>(shaderProgram, "lightDir");
		gl::UniformSampler(shaderProgram, "albedoTexture").Set(0);
		gl::UniformSampler(shaderProgram, "normalMap").Set(1);
		gl::UniformSampler(shaderProgram, "specularTexture").Set(2);
		gl::UniformSampler(shaderProgram, "roughnessTexture").Set(3);
	}
	catch (gl::Error& err) {
		std::cout << err.what() << std::endl;
	}
}

StandardMaterial::~StandardMaterial()
{}

StandardMaterial::StandardMaterial(StandardMaterial&& r) :
pGraphicsEngine(r.pGraphicsEngine),
albedoTexture(std::move(r.albedoTexture)),
normalMap(std::move(r.normalMap)),
specularTexture(std::move(r.specularTexture)),
roughnessTexture(std::move(r.roughnessTexture)),
sh_lightDir(std::move(r.sh_lightDir)),
sh_MVP(std::move(r.sh_MVP)),
sh_MODELVIEW(std::move(r.sh_MODELVIEW)),
sh_modelTransposedInverse(std::move(r.sh_modelTransposedInverse)),
shaderProgram(std::move(r.shaderProgram))
{}

StandardMaterial& StandardMaterial::operator=(StandardMaterial&& r)
{
	pGraphicsEngine = r.pGraphicsEngine;
	albedoTexture = std::move(r.albedoTexture);
	normalMap = std::move(r.normalMap);
	specularTexture = std::move(r.specularTexture);
	roughnessTexture = std::move(r.roughnessTexture);
	sh_lightDir = std::move(r.sh_lightDir);
	sh_MVP = std::move(r.sh_MVP);
	sh_MODELVIEW = std::move(r.sh_MODELVIEW);
	sh_modelTransposedInverse = std::move(r.sh_modelTransposedInverse);
	shaderProgram = std::move(r.shaderProgram);

	return *this;
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

void StandardMaterial::Prepare(Mesh::Submesh& submsh, const gl::Mat4f& modelTransform)
{
	Prepare(submsh);
	SetTransform(modelTransform);
}

void StandardMaterial::Prepare(Mesh::Submesh& submsh)
{
	//TODO might be too slow to set vertex attributes every time,
	//but if multiple meshes are drawn with a single  material it is necessary
	submsh.AttachVertexAttribute(AttributeCategory::POSITION, shaderProgram, "vertexPos");
	submsh.AttachVertexAttribute(AttributeCategory::NORMAL, shaderProgram, "vertexNormal");
	submsh.AttachVertexAttribute(AttributeCategory::TEX_COORD, shaderProgram, "vertexTexCoord");
	submsh.AttachVertexAttribute(AttributeCategory::TANGENT, shaderProgram, "vertexTangent");

	shaderProgram.Use();

	sh_lightDir.Set(pGraphicsEngine->GetSun().GetDirectionTowardsSource());

	gl::Texture::Active(0);
	albedoTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(1);
	normalMap.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(2);
	specularTexture.Bind(gl::Texture::Target::_2D);
	gl::Texture::Active(3);
	roughnessTexture.Bind(gl::Texture::Target::_2D);
}

void StandardMaterial::SetTransform(const gl::Mat4f& modelTransform)
{
	shaderProgram.Use();

	sh_MVP.Set(pGraphicsEngine->GetActiveCamera()->GetViewProjectionTransform() * modelTransform);
	sh_modelTransposedInverse.Set(MyTranspose(gl::Inverse(modelTransform)));
}
