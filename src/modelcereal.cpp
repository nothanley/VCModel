#include "modelcereal.h"
#include "BinaryIO.h"
#include "meshbuffers.h"
#include <meshtags.h>
#include "winsock.h"
#include "blendshapes.h"
#include <glm/gtx/euler_angles.hpp>

using namespace BinaryIO;
using namespace MeshSerializer;

CSerializedModel::CSerializedModel()
{
	m_data = nullptr;
	m_version = -1;
}

RigBone* CSerializedModel::loadBoneTransform(char*& buffer)
{
	/* Reads bone transformation matrix */
	RigBone* bone = new RigBone;
	glm::vec3 position(ReadFloat(buffer), ReadFloat(buffer), ReadFloat(buffer));
	glm::vec3 rotation(ReadFloat(buffer), ReadFloat(buffer), ReadFloat(buffer));

	bone->matrix_local = glm::eulerAngleZYX(rotation.x, rotation.y, rotation.z);
	bone->matrix_local[3] = glm::vec4(position.z, position.y, position.x, 1.0f);
	bone->matrix_world = bone->matrix_local; // Set transform basis
	return bone;
}


void CSerializedModel::getAxisAlignedBoundingBox(Mesh& mesh, bool getRadius) {
	BoundingBox& box = mesh.bounds;

	box.minX = ReadFloat(m_data);
	box.minY = ReadFloat(m_data);
	box.minZ = ReadFloat(m_data);
	box.maxX = ReadFloat(m_data);
	box.maxY = ReadFloat(m_data);
	box.maxZ = ReadFloat(m_data);
}

void CSerializedModel::loadStringTable()
{
	uint32_t numStrings = ReadUInt32(m_data);
	printf("\n\tStringTable size: %d", numStrings);

	/* Iterate and collect all string values */
	char* tablePointer = m_data + (numStrings * 0x4);
	for (int i = 0; i < numStrings; i++)
	{
		uintptr_t stringOffset = ReadUInt32(m_data);
		char* data = tablePointer + stringOffset;

		m_stringTable.push_back( std::string(data) );
	}
}

void CSerializedModel::loadMaterials()
{
	uint32_t numMats = ReadUInt32(m_data);

	for (int i = 0; i < numMats; i++) {
		Material mat;
		uint32_t index = ReadUInt32(m_data);
		mat.name = m_stringTable.at(index);

		m_materials.push_back(mat);
	}
}

void CSerializedModel::loadMeshDef(char* data, const MeshDefBf& mBuffer, Mesh& mesh)
{
	unsigned int hash = Data::hash(mBuffer.format.c_str());

	switch (hash)
	{
		case POSITION:
			Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.vertices);
			break;
		case NORMALS:
			Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.normals);
			break;
		case BINORMALS:
			Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.binormals);
			break;
		case TANGENTS:
			Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.tangents);
			break;
		case BLENDWEIGHTS:
			Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.skin.weights);
			break;
		case BLENDINDICES:
			Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.skin.indices);
			break;
		case TEXCOORDS:{
				UVMap uvChannel;
				Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, uvChannel.map);
				mesh.uvs.push_back(uvChannel);
			}
			break;
		case COLOR:{
				VertexColorSet set;
				Data::getDataSet(data, mesh.numVerts, mBuffer.type, mBuffer.property, set.map);
				mesh.colors.push_back(set);
			}
			break;
		default:
			break;
	}
}


void CSerializedModel::loadMeshData(Mesh& mesh)
{
	MeshDefBf mBuffer;
	mBuffer.format   = m_stringTable.at(ReadUInt16(m_data));
	mBuffer.property = m_stringTable.at(ReadUInt16(m_data));
	mBuffer.type	 = m_stringTable.at(ReadUInt16(m_data));
	m_data = Data::roundPointerToNearest4(m_data);

	/* Load mesh buffer charptr */
	uint32_t size = Data::getDataSetSize(mesh.numVerts, mBuffer.property);

	/* Collect and set mesh buffer data */
	this->loadMeshDef(m_data, mBuffer, mesh);
	m_data += size;
}

void CSerializedModel::getSkinData(Mesh& mesh)
{
	Skin& skin = mesh.skin;
	uint16_t jointEndIndex = ReadUInt16(m_data);
	skin.numWeights = ReadUInt16(m_data);
	if (skin.numWeights == 0) return;

	// iterate and collect indices and weights 
	for (int i = 0; i < mesh.numVerts; i++) {
		for (int j = 0; j < skin.numWeights; j++) {
			uint16_t index = ReadUInt16(m_data);
			skin.indices.push_back(index);
		}
	}
	m_data = Data::roundPointerToNearest4(m_data);

	for (int i = 0; i < mesh.numVerts; i++) {
		for (int j = 0; j < skin.numWeights; j++) {
			float weight = ( ReadUInt8(m_data) / 255.0 );
			skin.weights.push_back(weight);
		}
	}
	m_data = Data::roundPointerToNearest4(m_data);
}

void CSerializedModel::getVertexRemap(Mesh& mesh) 
{
	std::vector<uint32_t> remapVector;

	for (int i = 0; i < mesh.numVerts; i++) {
		uint32_t index = ReadUInt32(m_data);
		remapVector.push_back(index);
	}
}


void CSerializedModel::getTriangleBuffer(Mesh& mesh)
{
	std::vector<int> indices;
	uint16_t index = ReadUInt16(m_data);
	uint32_t numIndices = ReadUInt32(m_data);
	int numTriangles = numIndices / 3;
	
	/* Collect all face indices */
	for (int i = 0; i < numTriangles; i++) {
		Triangle face = (mesh.numVerts > 65535) ?
			Triangle{ ReadUInt32(m_data), ReadUInt32(m_data), ReadUInt32(m_data) } :
			Triangle{ ReadUInt16(m_data), ReadUInt16(m_data), ReadUInt16(m_data) };

		mesh.triangles.push_back(face);
	}

	/* Assign per face materials */
	m_data = Data::roundPointerToNearest4(m_data);
	uint32_t numMaterials = ReadUInt32(m_data);

	/* Collect all material face groups */
	for (int i = 0; i < numMaterials; i++) 
	{
		FaceGroup mtlGroup;
		uint32_t mtlIndex	  = ReadUInt32(m_data);
		mtlGroup.faceBegin	  = ReadUInt32(m_data) / 3;
		mtlGroup.numTriangles = ReadUInt32(m_data) / 3;
		mtlGroup.material     = m_materials.at(mtlIndex);

		mesh.groups.push_back(mtlGroup);
	}

	m_data += 0x8;
}

void CSerializedModel::loadLods()
{
	uint32_t numFaceBufs = ReadUInt32(m_data);
	uint32_t lodCap = (true) ? 1 : numFaceBufs;

	/* Iterate through face buffers and collect lods and material sets */
	for (int i = 0; i < lodCap; i++){
		uint32_t numIndices = ReadUInt32(m_data);
		for (int j = 0; j < numIndices; j++)
			this->getTriangleBuffer(*m_meshes.at(j));
	}
}

void CSerializedModel::loadColorMapInfo(Mesh& mesh)
{
	for (auto& colorMap : mesh.colors) {
		int index = ReadUInt32(m_data);
		colorMap.name = m_stringTable.at(index);
	}
}

void CSerializedModel::loadUVInfo(Mesh& mesh)
{
	for (auto& map : mesh.uvs) {
		int index = ReadUInt32(m_data);
		map.name = m_stringTable.at(index);
	}
}

void CSerializedModel::getMorphWeights(Mesh& mesh)
{
	/* Load all vertex morphs */
	vCMeshShapes blendshapes(m_data, m_stringTable, &mesh);
	blendshapes.load();
}

void CSerializedModel::buildMesh(Mesh& mesh)
{
	uint32_t index, numStacks;
	index = ReadUInt32(m_data);

	mesh.sceneFlag = ReadUInt32(m_data);
	m_data	+= sizeof(uint16_t); // null const
	mesh.motionFlag = ReadUInt32(m_data);
	getAxisAlignedBoundingBox(mesh);

	mesh.numVerts = ReadUInt32(m_data);
	numStacks = ReadUInt32(m_data);
	mesh.name = m_stringTable.at(index);

	for (int j = 0; j < numStacks; j++) {
		uint32_t dataMagic = ReadUInt32(m_data);
		uint32_t typeMagic = ReadUInt32(m_data);
		uint32_t formatMagic = ReadUInt32(m_data);
		this->loadMeshData(mesh);
	}

	this->getSkinData(mesh);
	this->getVertexRemap(mesh);
	this->getMorphWeights(mesh);
	this->getMeshMapInfo(mesh);
}

void CSerializedModel::loadMeshes()
{
	uint32_t numModels = ReadUInt32(m_data);
	printf("\n\tTotal Models: %d\n", numModels);

	for (int i = 0; i < numModels; i++) {
		Mesh* mesh = new Mesh;

		this->buildMesh(*mesh);
		m_meshes.push_back(mesh);
		//printf("\n\tRead Model: %s\n", mesh->name.c_str());
	}
}
