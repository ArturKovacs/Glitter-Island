#include <GE/MeshManager.hpp>

#include <cassert>
#include <cctype>
#include <sstream>
#include <iostream>

#include <GE/GraphicsEngine.hpp>

util::managed_ptr<Mesh> MeshManager::LoadMeshFromFile(GraphicsEngine* pGraphicsEngine, const std::string& filename)
{
	auto elementIter = meshes.find(filename);
	if (elementIter != meshes.end()) {
		return (*elementIter).second;
	}

	std::cout << "Loading mesh!" << std::endl;

	Mesh* pResult = LoadFromOBJFile(pGraphicsEngine, filename);

	std::cout << "Mesh loaded!" << std::endl;

	meshes[filename] = pResult;

	return pResult;
}

template<typename elementType>
static std::vector<GLfloat> GetFloatVector(const std::vector<elementType>& input, int inputDimensions)
{
	constexpr bool is_vec2 = std::is_same<elementType, glm::vec2>::value;
	constexpr bool is_vec3 = std::is_same<elementType, glm::vec3>::value;
	static_assert(is_vec2 || is_vec3, "Unsupported type!");

	constexpr int elementCount = is_vec2 ? 2 : 3;

	assert(inputDimensions <= elementCount);

	std::vector<GLfloat> result(input.size()*inputDimensions);

	int resultIndex = 0;
	for (const auto& current : input) {
		for (int i = 0; i < inputDimensions; i++) {
			result.at(resultIndex++) = current[i];
		}
	}

	return result;
}

template<typename itemType>
static std::vector<itemType> get_values_str(const std::string& str)
{
	std::stringstream ss(str);
	std::vector<itemType> result;
	itemType tmpItem;
	while (ss >> tmpItem) {
		result.push_back(std::move(tmpItem));
	}

	return result;
}

static std::vector<int> get_attrib_indices_from_vertex(const std::string& str)
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
			vertexAttributes.insert(std::make_pair(current, std::vector<glm::vec3>()));
		});
	}

	std::vector<Mesh::Submesh::IndexType> indices;
	std::map<AttributeCategory,  std::vector<glm::vec3> > vertexAttributes;
};

Mesh* MeshManager::LoadFromOBJFile(GraphicsEngine* pGraphicsEngine, const std::string& filename)
{
	Mesh* pResult = new Mesh;

	std::ifstream objFile(filename);
	if (!objFile.is_open()) {
		throw std::runtime_error(std::string("Could not open obj file: ") + filename);
	}

	using MaterialName = std::string;
	std::map<MaterialName, OBJSubmesh> obj_submeshes;

	const MaterialName undefinedMtlName = "?";
	MaterialName currMtlName = undefinedMtlName;

	std::string mtllibFilename;

	std::map<AttributeCategory, std::vector<glm::vec3> > attribLists;
	attribLists.insert(std::make_pair(AttributeCategory::POSITION, std::vector<glm::vec3>()));
	attribLists.insert(std::make_pair(AttributeCategory::TEX_COORD, std::vector<glm::vec3>()));
	attribLists.insert(std::make_pair(AttributeCategory::NORMAL, std::vector<glm::vec3>()));

	std::string read;
	std::string values_str;

	auto start = objFile.tellg();
	objFile.seekg(0, std::ios::end);
	double length = double(objFile.tellg() - start);
	objFile.seekg(start);

	int displayRes = 10;
	int displayedProgress = 0;

	auto ReadAndPutAttributeToList = [&](AttributeCategory attrib){
		std::getline(objFile, values_str);
		auto coordinates = get_values_str<GLfloat>(values_str);
		glm::vec3 attribData(0, 0, 0);
		constexpr int numComponents = 3;

		assert(coordinates.size() <= numComponents);

		for (size_t i = 0; i < coordinates.size(); i++) {
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
				catch (std::out_of_range&){}
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

		double currDiff = double(objFile.tellg() - start);
		double currentProgress = double((currDiff / length)*displayRes);
		while (currentProgress - displayedProgress > 0) {
			std::cout << '-' << displayRes-displayedProgress;
			displayedProgress++;
		}
		std::cout.flush();
	}

	std::cout << std::endl;

	//calculate tangents
	for (auto& current : obj_submeshes) {
		OBJSubmesh& currentMesh = current.second;
		if (currentMesh.vertexAttributes.at(AttributeCategory::TEX_COORD).size() > 0) {
			const auto& actualPosData = currentMesh.vertexAttributes.at(AttributeCategory::POSITION);
			const auto& actualTexCoordData = currentMesh.vertexAttributes.at(AttributeCategory::TEX_COORD);
			for (size_t i = 0; i < actualPosData.size(); i += 3) {
				const auto& curr = actualPosData.at(i);
				const auto& next = actualPosData.at(i + 1);
				const auto& nextnext = actualPosData.at(i + 2);

				glm::vec2 deltaUV1 = glm::vec2(actualTexCoordData.at(i + 1)) - glm::vec2(actualTexCoordData.at(i));
				glm::vec2 deltaUV2 = glm::vec2(actualTexCoordData.at(i + 2)) - glm::vec2(actualTexCoordData.at(i));

				float dU1 = deltaUV1.x;
				float dU2 = deltaUV2.x;
				float dV1 = deltaUV1.y;
				float dV2 = deltaUV2.y;

				glm::vec3 edge1 = next - curr;
				glm::vec3 edge2 = nextnext - curr;

				glm::vec3 tangent = (dV2*edge1 - dV1*edge2) / (dU1*dV2 - dU2*dV1);

				for (int i = 0; i < 3; i++) {
					currentMesh.vertexAttributes.at(AttributeCategory::TANGENT).push_back(tangent);
				}
			}
		}
	}

	for (auto& currSrc : obj_submeshes) {
		OBJSubmesh& currOBJSubmesh = currSrc.second;
		const MaterialName& currMaterialName = currSrc.first;

		Mesh::Submesh currDst;

		currDst.SetMaterial(pGraphicsEngine->LoadStandardMaterialFromFile(mtllibFilename, currMaterialName).get_raw());
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

		pResult->submeshes.push_back(std::move(currDst));
	}

	return pResult;
}
