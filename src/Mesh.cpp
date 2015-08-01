#include "Mesh.hpp"

#include <limits>
#include <string>
#include <sstream>

Mesh Mesh::GenerateTriangle(float size)
{
	Mesh result;

	std::vector<GLfloat> vertexPos = {
		0.0f, 0.0f, 0.0f,
		size, 0.0f, 0.0f,
		0.0f, size, 0.0f
	};
	result.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPos);

	std::vector<GLfloat> vertexNormal = {
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f
	};
	result.SetVertexAttributeBuffer(AttributeCategory::NORMAL, vertexNormal);

	std::vector<IndexType> indexData = {0, 1, 2};
	result.SetIndices(indexData);

	result.SetPrimitiveType(gl::enums::PrimitiveType::Triangles);

	return std::move(result);
}

Mesh Mesh::GenerateCircle(float radius, int resolution)
{
	Mesh result;

	std::vector<gl::Vec2f> vertexPosData(resolution);
	for (int i = 0; i < resolution; i++) {
		float currAngle = (float(i)/resolution)*2*gl::math::Pi();
		vertexPosData.at(i) = gl::Vec2f(std::cos(currAngle)*radius, std::sin(currAngle)*radius);
	}
	result.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);
	result.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPosData);

	std::vector<IndexType> indexData(resolution);
	for (int i = 0; i < resolution; i++) {
		indexData.at(i) = i;
	}
	result.SetIndices(indexData);

	result.SetPrimitiveType(gl::enums::PrimitiveType::LineLoop);

	return std::move(result);
}

Mesh Mesh::GenerateRectangle(float sizeX, float sizeY)
{
	Mesh result;

	std::vector<gl::Vec2f> vertexPos = {
		gl::Vec2f(-sizeX*0.5f, -sizeY*0.5f),
		gl::Vec2f(+sizeX*0.5f, -sizeY*0.5f),
		gl::Vec2f(+sizeX*0.5f, +sizeY*0.5f),
		gl::Vec2f(-sizeX*0.5f, +sizeY*0.5f),
	};
	result.SetVertexAttributeBuffer(AttributeCategory::POSITION, vertexPos);
	result.SetVertexAttributeElementDimensions(AttributeCategory::POSITION, 2);

	std::vector<gl::Vec3f> vertexNormal = {
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
		gl::Vec3f(0, 0, 1),
	};
	result.SetVertexAttributeBuffer(AttributeCategory::NORMAL, vertexNormal);
	result.SetVertexAttributeElementDimensions(AttributeCategory::NORMAL, 3);

	std::vector<IndexType> indexData = {0, 1, 2, 3};
	result.SetIndices(indexData);

	result.SetPrimitiveType(gl::enums::PrimitiveType::TriangleFan);

	return std::move(result);
}

Mesh::Mesh() : primitiveType(gl::enums::PrimitiveType::Points)
{
	ForEachAttribute([&](AttributeCategory current){
		vertexAttributes.insert(std::make_pair(current, VertexAttributeContainer(std::move(gl::Buffer()), GetAttributeCategoryInfo(current).defaultElementDimensions)));
	});
}

Mesh::Mesh(Mesh&& r) :
VAO(std::move(r.VAO)),
indices(std::move(r.indices)),
vertexAttributes(std::move(r.vertexAttributes)),
primitiveType(r.primitiveType)
{}

Mesh& Mesh::operator=(Mesh&& r)
{
	VAO = std::move(r.VAO);
	indices = std::move(r.indices);
	vertexAttributes = std::move(r.vertexAttributes);
	primitiveType = r.primitiveType;

	return *this;
}

void Mesh::SetIndices(const std::vector<IndexType>& indexArray)
{
	VAO.Bind();

	indices.Bind(gl::Buffer::Target::ElementArray);
	gl::Buffer::Data(gl::Buffer::Target::ElementArray, indexArray);
}

void Mesh::SetPrimitiveType(gl::enums::PrimitiveType type)
{
	primitiveType = type;
}

void Mesh::SetVertexAttributeElementDimensions(AttributeCategory targetAttribute, const int dimensionCount)
{
	vertexAttributes.at(targetAttribute).elementDimension = dimensionCount;
}

void Mesh::SetVertexAttributeBuffer(AttributeCategory targetAttribute, const gl::BufferData& dataArray)
{
	VAO.Bind();

	vertexAttributes.at(targetAttribute).buffer.Bind(gl::Buffer::Target::Array);
	gl::Buffer::Data(gl::Buffer::Target::Array, dataArray);
}

gl::enums::PrimitiveType Mesh::GetPrimitiveType() const
{
	return primitiveType;
}

GLsizei Mesh::GetNumOfIndices() const
{
	return indices.Size(gl::Buffer::Target::ElementArray).get() / sizeof(IndexType);
}

void Mesh::AttachVertexAttribute(const AttributeCategory targetAttribute, const gl::Program& shaderProgram, const std::string& nameInShader) const
{
	const auto& attributeContainer = vertexAttributes.at(targetAttribute);

	VAO.Bind();
	attributeContainer.buffer.Bind(gl::Buffer::Target::Array);
	gl::VertexArrayAttrib attribute(shaderProgram, nameInShader);
	attribute.Setup<GLfloat>(attributeContainer.elementDimension);
	attribute.Enable();
}

void Mesh::BindVAO() const
{
	VAO.Bind();
}

/*
template<typename itemType>
std::vector<itemType> split_str(const std::string& str, char delim)
{
std::stringstream ss(str);
std::vector<itemType> result;
std::string part;
while (std::getline(ss, part, delim)) {
std::stringstream part_ss(part);
itemType tmpItem;
part_ss >> tmpItem;
result.push_back(std::move(tmpItem));
}

return result;
}*/

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

void Mesh::LoadOBJMeshFromFile(const std::string& filename)
{
	std::cout << "Loading mesh!" << std::endl;

	std::ifstream objFile(filename);
	if (!objFile.is_open()) {
		throw std::runtime_error(std::string("Could not open obj file: ") + filename);
	}

	std::vector<Mesh::IndexType> obj_indices;
	std::map<AttributeCategory,  std::vector<gl::Vec3f> > obj_vertexAttributes;

	ForEachAttribute([&](AttributeCategory current){
		obj_vertexAttributes.insert(std::make_pair(current, std::vector<gl::Vec3f>()));
    });

	std::map<AttributeCategory, std::vector<gl::Vec3f> > attribLists;
	attribLists.insert(std::make_pair(AttributeCategory::POSITION, std::vector<gl::Vec3f>()));
	attribLists.insert(std::make_pair(AttributeCategory::TEX_COORD, std::vector<gl::Vec3f>()));
	attribLists.insert(std::make_pair(AttributeCategory::NORMAL, std::vector<gl::Vec3f>()));

	int currIndex = 0;

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
		values_str = values_str.substr(0, values_str.find('#'));
		auto coordinates = get_values_str<GLfloat>(values_str);
		gl::Vec3f attribData(0, 0, 0);

		assert(coordinates.size() <= attribData.Size());

		for (int i = 0; i < coordinates.size(); i++) {
			attribData[i] = coordinates[i];
		}
		attribLists.at(attrib).push_back(attribData);
	};

	auto CopyAttributeFromListToActualData = [&](AttributeCategory attrib, int OBJindex){
		obj_vertexAttributes.at(attrib).push_back(attribLists.at(attrib).at(OBJindex-1));
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
			values_str = values_str.substr(0, values_str.find('#'));
			auto faceVertices = get_values_str<std::string>(values_str);
			if (faceVertices.size() != 3) {
				throw std::runtime_error("Error loading obj file!\nReason: model has at least one face that is not a triangle.\nOnly triangle meshes are supported!");
			}
			for (auto& currentVertex : faceVertices) {
				auto vertexAttribIndices = get_attrib_indices_from_vertex(currentVertex);
				obj_indices.push_back(currIndex++);
				int posListIndex = vertexAttribIndices.at(0);
				CopyAttributeFromListToActualData(AttributeCategory::POSITION, posListIndex);
				try {
					int texCoordListIndex = vertexAttribIndices.at(1);
					if (texCoordListIndex > 0) {
						CopyAttributeFromListToActualData(AttributeCategory::TEX_COORD, texCoordListIndex);
					}
					int normalListIndex = vertexAttribIndices.at(2);
					if (normalListIndex > 0) {
						CopyAttributeFromListToActualData(AttributeCategory::NORMAL, normalListIndex);
					}
				}
				catch (std::out_of_range& ex){}
			}
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

	if (obj_vertexAttributes.at(AttributeCategory::TEX_COORD).size() > 0) {
		//calculate tangents
		const auto& actualPosData = obj_vertexAttributes.at(AttributeCategory::POSITION);
		const auto& actualTexCoordData = obj_vertexAttributes.at(AttributeCategory::TEX_COORD);
		for (int i = 0; i < actualPosData.size(); i += 3) {
			const auto& curr = actualPosData.at(i);
			const auto& next = actualPosData.at(i + 1);
			const auto& nextnext = actualPosData.at(i + 2);

			gl::Vec2f deltaUV1 = actualTexCoordData.at(i+1).xy() - actualTexCoordData.at(i).xy();
			gl::Vec2f deltaUV2 = actualTexCoordData.at(i+2).xy() - actualTexCoordData.at(i).xy();

			float dU1 = deltaUV1.x();
			float dU2 = deltaUV2.x();
			float dV1 = deltaUV1.y();
			float dV2 = deltaUV2.y();

			gl::Vec3f edge1 = next - curr;
			gl::Vec3f edge2 = nextnext - curr;

			gl::Vec3f tangent = (dV2*edge1 - dV1*edge2)/(dU1*dV2 - dU2*dV1);

			for (int i = 0; i < 3; i++) {
				obj_vertexAttributes.at(AttributeCategory::TANGENT).push_back(tangent);
			}
		}
	}

	SetIndices(obj_indices);

	auto GetOBJVertexAttribute = [&](AttributeCategory target) {
		return GetFloatVector(obj_vertexAttributes.at(target), GetAttributeCategoryInfo(target).defaultElementDimensions);
	};

	ForEachAttribute([&](AttributeCategory current){
		auto attributeBuffer = GetOBJVertexAttribute(current);
		if (attributeBuffer.size() > 0) {
			SetVertexAttributeBuffer(current, attributeBuffer);
		}
	});

	SetPrimitiveType(gl::PrimitiveType::Triangles);

	std::cout << "Mesh loaded!" << std::endl;
}
/*
OBJMesh::OBJMesh(std::istream& objContent)
{
	
}

std::vector<Mesh::IndexType>& OBJMesh::GetIndices()
{
	return indices;
}

std::vector<GLfloat> OBJMesh::GetVertexAttribute(AttributeCategory target)
{
	return GetFloatVector(vertexAttributes.at(target), GetAttributeCategoryInfo(target).defaultElementDimensions);
}

*/
