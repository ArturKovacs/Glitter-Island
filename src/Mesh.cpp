#include "Mesh.hpp"

#include <limits>
#include <string>
#include <sstream>
#include <functional>
#include <cctype>

#include "DemoCore.hpp"

Mesh::Mesh(){}

Mesh Mesh::GenerateTriangle(float size)
{
	Mesh result;
	Submesh submsh;

	std::vector<GLfloat> vertexPos = {
		0.0f, 0.0f, 0.0f,
		size, 0.0f, 0.0f,
		0.0f, size, 0.0f
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPos);

	std::vector<GLfloat> vertexNormal = {
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, vertexNormal);

	std::vector<Submesh::IndexType> indexData = {0, 1, 2};
	submsh.SetIndices(indexData);

	submsh.SetPrimitiveType(gl::enums::PrimitiveType::Triangles);

	result.submeshes.push_back(std::move(submsh));
	return std::move(result);
}

Mesh Mesh::GenerateCircle(float radius, int resolution)
{
	Mesh result;
	Submesh submsh;

	std::vector<gl::Vec2f> vertexPosData(resolution);
	for (int i = 0; i < resolution; i++) {
		float currAngle = (float(i)/resolution)*2*gl::math::Pi();
		vertexPosData.at(i) = gl::Vec2f(std::cos(currAngle)*radius, std::sin(currAngle)*radius);
	}
	submsh.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);
	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPosData);

	std::vector<Submesh::IndexType> indexData(resolution);
	for (int i = 0; i < resolution; i++) {
		indexData.at(i) = i;
	}
	submsh.SetIndices(indexData);

	submsh.SetPrimitiveType(gl::enums::PrimitiveType::LineLoop);

	result.submeshes.push_back(std::move(submsh));
	return std::move(result);
}

Mesh Mesh::GenerateRectangle(float sizeX, float sizeY)
{
	Mesh result;
	Submesh submsh;

	std::vector<gl::Vec2f> vertexPos = {
		gl::Vec2f(-sizeX*0.5f, -sizeY*0.5f),
		gl::Vec2f(+sizeX*0.5f, -sizeY*0.5f),
		gl::Vec2f(+sizeX*0.5f, +sizeY*0.5f),
		gl::Vec2f(-sizeX*0.5f, +sizeY*0.5f),
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPos);
	submsh.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);

	std::vector<gl::Vec3f> vertexNormal = {
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
	};
	submsh.SetVertexAttributeBuffer(AttributeCategory::NORMAL, vertexNormal);
	submsh.SetVertexAttributeElementDimensions(AttributeCategory::NORMAL, 3);

	std::vector<Submesh::IndexType> indexData = {0, 1, 2, 3};
	submsh.SetIndices(indexData);

	submsh.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);

	result.submeshes.push_back(std::move(submsh));
	return std::move(result);
}

Mesh::Mesh(Mesh&& r) :
submeshes(std::move(r.submeshes))
{}

Mesh& Mesh::operator=(Mesh&& r)
{
	submeshes = std::move(r.submeshes);

	return *this;
}

std::vector<Mesh::Submesh>& Mesh::GetSubmeshes()
{
	return submeshes;
}

const std::vector<Mesh::Submesh>& Mesh::GetSubmeshes() const
{
	return submeshes;
}

/*
const std::vector<std::string>& Mesh::GetMtlLibFilenames() const
{
	return mtllibFilenames;
}
*/

template<typename itemType>
std::vector<itemType> get_values_str(const std::string& str)
{
	std::stringstream ss(str);
	std::vector<itemType> result;
	itemType tmpItem;
	while (ss >> tmpItem) {
		result.push_back(std::move(tmpItem));
	}

	return result;
}

std::vector<int> get_attrib_indices_from_vertex(const std::string& str)
{
	char delim = '/';
	std::stringstream ss(str);
	std::vector<int> result;
	std::string part;
	while (std::getline(ss, part, delim)) {
		int attribIndex = 0;
		if (part != "") {
			std::stringstream part_ss(part);
			part_ss >> attribIndex;

			//Objs sometimes contain negative indices. Nonsense!
			attribIndex = std::abs(attribIndex);
		}

		result.push_back(attribIndex);
	}

	return result;
}

struct OBJSubmesh
{
	OBJSubmesh()
	{
		ForEachAttribute([&](AttributeCategory current){
			vertexAttributes.insert(std::make_pair(current, std::vector<gl::Vec3f>()));
		});
	}

	std::vector<Mesh::Submesh::IndexType> indices;
	std::map<AttributeCategory,  std::vector<gl::Vec3f> > vertexAttributes;
};

void Mesh::LoadFromOBJFile(DemoCore* pCore, const std::string& filename)
{
	std::cout << "Loading mesh!" << std::endl;

	std::ifstream objFile(filename);
	if (!objFile.is_open()) {
		throw std::runtime_error(std::string("Could not open obj file: ") + filename);
	}

	using MaterialName = std::string;
	std::map<MaterialName, OBJSubmesh> obj_submeshes;

	const MaterialName undefinedMtlName = "?";
	MaterialName currMtlName = undefinedMtlName;

	std::string mtllibFilename;

	std::map<AttributeCategory, std::vector<gl::Vec3f> > attribLists;
	attribLists.insert(std::make_pair(AttributeCategory::POSITION, std::vector<gl::Vec3f>()));
	attribLists.insert(std::make_pair(AttributeCategory::TEX_COORD, std::vector<gl::Vec3f>()));
	attribLists.insert(std::make_pair(AttributeCategory::NORMAL, std::vector<gl::Vec3f>()));

	//int currIndex = 0;

	std::string read;
	std::string values_str;

	auto start = objFile.tellg();
	objFile.seekg(0, std::ios::end);
	float length = objFile.tellg() - start;
	objFile.seekg(start);

	int displayRes = 10;
	int displayedProgress = 0;

	auto ReadAndPutAttributeToList = [&](AttributeCategory attrib){
		std::getline(objFile, values_str);
		auto coordinates = get_values_str<GLfloat>(values_str);
		gl::Vec3f attribData(0, 0, 0);

		assert(coordinates.size() <= attribData.Size());

		for (int i = 0; i < coordinates.size(); i++) {
			attribData[i] = coordinates[i];
		}
		attribLists.at(attrib).push_back(attribData);
	};

	auto CopyAttributeFromListToActualData = [&](OBJSubmesh& target, AttributeCategory attrib, int OBJindex){
		target.vertexAttributes.at(attrib).push_back(attribLists.at(attrib).at(OBJindex-1));
	};

	while (objFile >> read) {
		if (read == "v") {
			ReadAndPutAttributeToList(AttributeCategory::POSITION);
		}
		else if (read == "vt") {
			ReadAndPutAttributeToList(AttributeCategory::TEX_COORD);
		}
		else if (read == "vn") {
			ReadAndPutAttributeToList(AttributeCategory::NORMAL);
		}
		else if (read == "f") {
			std::getline(objFile, values_str);
			auto faceVertices = get_values_str<std::string>(values_str);
			if (faceVertices.size() != 3) {
				throw std::runtime_error("Error loading obj file!\nReason: model has at least one face that is not a triangle.\nOnly triangle meshes are supported!");
			}

			OBJSubmesh& currSubmesh = obj_submeshes[currMtlName];

			for (auto& currentVertex : faceVertices) {
				auto vertexAttribIndices = get_attrib_indices_from_vertex(currentVertex);
				currSubmesh.indices.push_back(currSubmesh.indices.size());
				int posListIndex = vertexAttribIndices.at(0);
				CopyAttributeFromListToActualData(currSubmesh, AttributeCategory::POSITION, posListIndex);
				try {
					int texCoordListIndex = vertexAttribIndices.at(1);
					if (texCoordListIndex > 0) {
						CopyAttributeFromListToActualData(currSubmesh, AttributeCategory::TEX_COORD, texCoordListIndex);
					}
					int normalListIndex = vertexAttribIndices.at(2);
					if (normalListIndex > 0) {
						CopyAttributeFromListToActualData(currSubmesh, AttributeCategory::NORMAL, normalListIndex);
					}
				}
				catch (std::out_of_range& ex){}
			}
		}
		else if (read == "usemtl") {
			objFile >> currMtlName;
		}
		else if (read == "mtllib") {
			std::getline(objFile, values_str);
			mtllibFilename = values_str;
			std::string& str = mtllibFilename;

			str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int c){return std::isspace(c) == false;}));
		}
		else {
			objFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}

		float currDiff = objFile.tellg() - start;
		float currentProgress = (currDiff / length)*displayRes;
		while (currentProgress - displayedProgress > 0) {
			std::cout << '-' << displayRes-displayedProgress;
			std::cout.flush();
			displayedProgress++;
		}
	}

	std::cout << std::endl;

	//calculate tangents
	for (auto& current : obj_submeshes) {
		OBJSubmesh& currentMesh = current.second;
		if (currentMesh.vertexAttributes.at(AttributeCategory::TEX_COORD).size() > 0) {
			const auto& actualPosData = currentMesh.vertexAttributes.at(AttributeCategory::POSITION);
			const auto& actualTexCoordData = currentMesh.vertexAttributes.at(AttributeCategory::TEX_COORD);
			for (int i = 0; i < actualPosData.size(); i += 3) {
				const auto& curr = actualPosData.at(i);
				const auto& next = actualPosData.at(i + 1);
				const auto& nextnext = actualPosData.at(i + 2);

				gl::Vec2f deltaUV1 = actualTexCoordData.at(i + 1).xy() - actualTexCoordData.at(i).xy();
				gl::Vec2f deltaUV2 = actualTexCoordData.at(i + 2).xy() - actualTexCoordData.at(i).xy();

				float dU1 = deltaUV1.x();
				float dU2 = deltaUV2.x();
				float dV1 = deltaUV1.y();
				float dV2 = deltaUV2.y();

				gl::Vec3f edge1 = next - curr;
				gl::Vec3f edge2 = nextnext - curr;

				gl::Vec3f tangent = (dV2*edge1 - dV1*edge2) / (dU1*dV2 - dU2*dV1);

				for (int i = 0; i < 3; i++) {
					currentMesh.vertexAttributes.at(AttributeCategory::TANGENT).push_back(tangent);
				}
			}
		}
	}

	for (auto& currSrc : obj_submeshes) {
		OBJSubmesh& currOBJSubmesh = currSrc.second;
		const MaterialName &currMaterialName = currSrc.first;

		Submesh currDst;

		//currDst.SetMaterialName(currSrc.first);
		currDst.SetMaterial(pCore->LoadStandardMaterialFromFile(mtllibFilename, currMaterialName));
		currDst.SetIndices(currOBJSubmesh.indices);

		auto GetOBJVertexAttribute = [&](AttributeCategory target) {
			return GetFloatVector(currOBJSubmesh.vertexAttributes.at(target), GetAttributeCategoryInfo(target).defaultElementDimensions);
		};

		ForEachAttribute([&](AttributeCategory current) {
			auto attributeBuffer = GetOBJVertexAttribute(current);
			if (attributeBuffer.size() > 0) {
				currDst.SetVertexAttributeBuffer(current, attributeBuffer);
			}
		});

		currDst.SetPrimitiveType(gl::PrimitiveType::Triangles);

		submeshes.push_back(std::move(currDst));
	}

	std::cout << "Mesh loaded!" << std::endl;
}


////////////////////////////////////////////////
//
// Mesh::Submesh
//
////////////////////////////////////////////////

Mesh::Submesh::Submesh() : pMaterial(nullptr), primitiveType(gl::enums::PrimitiveType::Points)
{
	ForEachAttribute([&](AttributeCategory current){
		vertexAttributes.insert(std::make_pair(current, VertexAttributeContainer(std::move(gl::Buffer()), GetAttributeCategoryInfo(current).defaultElementDimensions)));
	});
}

Mesh::Submesh::Submesh(Submesh&& r) :
pMaterial(r.pMaterial),
VAO(std::move(r.VAO)),
indices(std::move(r.indices)),
vertexAttributes(std::move(r.vertexAttributes)),
primitiveType(r.primitiveType)
{}

Mesh::Submesh& Mesh::Submesh::operator=(Submesh&& r)
{
	pMaterial = r.pMaterial;
	VAO = std::move(r.VAO);
	indices = std::move(r.indices);
	vertexAttributes = std::move(r.vertexAttributes);
	primitiveType = r.primitiveType;

	return *this;
}

void Mesh::Submesh::SetIndices(const std::vector<IndexType>& indexArray)
{
	VAO.Bind();

	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexArray);
}

void Mesh::Submesh::SetPrimitiveType(gl::enums::PrimitiveType type)
{
	primitiveType = type;
}

void Mesh::Submesh::SetVertexAttributeElementDimensions(AttributeCategory targetAttribute, const int dimensionCount)
{
	vertexAttributes.at(targetAttribute).elementDimension = dimensionCount;
}

void Mesh::Submesh::SetVertexAttributeBuffer(AttributeCategory targetAttribute, const gl::BufferData& dataArray)
{
	VAO.Bind();

	vertexAttributes.at(targetAttribute).buffer.Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, dataArray);
}

void Mesh::Submesh::SetMaterial(Material* newMaterial)
{
	pMaterial = newMaterial;
}

Material* Mesh::Submesh::GetMaterial()
{
	return pMaterial;
}

gl::enums::PrimitiveType Mesh::Submesh::GetPrimitiveType() const
{
	return primitiveType;
}

GLsizei Mesh::Submesh::GetNumOfIndices() const
{
	return indices.Size(gl::Buffer::Target::ElementArray).get() / sizeof(IndexType);
}

void Mesh::Submesh::AttachVertexAttribute(const AttributeCategory targetAttribute, const gl::Program& shaderProgram, const std::string& nameInShader) const
{
	const auto& attributeContainer = vertexAttributes.at(targetAttribute);

	VAO.Bind();
	attributeContainer.buffer.Bind(gl::Buffer::Target::Array);
	gl::VertexArrayAttrib attribute(shaderProgram, nameInShader);
	attribute.Setup<GLfloat>(attributeContainer.elementDimension);
	attribute.Enable();
}

void Mesh::Submesh::BindVAO() const
{
	VAO.Bind();
}
