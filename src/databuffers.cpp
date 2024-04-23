#include "databuffers.h"
#include "BinaryIO.h"
#include "meshbuffers.h"
#include <meshtags.h>
#include "winsock.h"
#include "meshshapes.h"
#include <glm/gtx/euler_angles.hpp>

using namespace BinaryIO;
using namespace MeshSerializer;

inline
RigBone* loadBoneTransform(char*& buffer)
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

inline 
void readBone2_8(char*& buffer, 
	const std::vector<std::string>& stringTable, std::vector<RigBone*>& bones) 
{
	int16_t index = ReadInt16(buffer);
	int16_t parentIndex = ReadInt16(buffer);
	bool isTypeJoint = !(index == 0 && parentIndex == 0);

	/* Get bone transformation matrix */
	RigBone* bone = loadBoneTransform(buffer);
	bone->index   = index;
	int8_t  unkValueA = ReadUInt8(buffer);  /* Perhaps a flag? */
	int32_t unkValueB = ReadUInt32(buffer); /* Unknown dword value */

	if (unkValueB != -1)
		buffer += 0x20;

	/* Filter irregular joint types */
	if (!isTypeJoint) {
		delete bone;
		return; }
	
	bones.at(index) = (isTypeJoint) ? bone : nullptr;
	bone->name = stringTable.at(index);

	/* Update bone hierarchy */
	if (parentIndex != -1)
		bone->set_parent(bones.at(parentIndex));
}

inline
void readBone2_5(char*& buffer,
	const std::vector<std::string>& stringTable, std::vector<RigBone*>& bones)
{
	int16_t index = ReadInt16(buffer);
	int16_t parentIndex = ReadInt16(buffer);
	bool isTypeJoint = !(index == 0 && parentIndex == 0);

	/* Get bone transformation matrix */
	RigBone* bone   = loadBoneTransform(buffer);
	bone->index     = index;
	buffer += sizeof(uint32_t);

	/* Filter irregular joint types */
	if (!isTypeJoint) {
		delete bone;
		return; }

	bones.at(index) = (isTypeJoint) ? bone : nullptr;
	bone->name = stringTable.at(index);

	/* Update bone hierarchy */
	if (parentIndex != -1)
		bone->set_parent(bones.at(parentIndex));
}



void getAxisAlignedBoundingBox(char*& buffer, Mesh& mesh, bool getRadius=true) {
	BoundingBox& box = mesh.bounds;

	box.minX = ReadFloat(buffer);
	box.minY = ReadFloat(buffer);
	box.minZ = ReadFloat(buffer);
	box.maxX = ReadFloat(buffer);
	box.maxY = ReadFloat(buffer);
	box.maxZ = ReadFloat(buffer);
}

void 
CDataBuffer::getStringTable(char* buffer, std::vector<std::string>& stringTable)
{
	uint32_t numStrings = ReadUInt32(buffer);
	printf("\n\tStringTable size: %d", numStrings);

	/* Iterate and collect all string values */
	char* tablePointer = buffer + (numStrings * 0x4);
	for (int i = 0; i < numStrings; i++)
	{
		uintptr_t stringOffset = ReadUInt32(buffer);
		char* data = tablePointer + stringOffset;

		stringTable.push_back( std::string(data) );
	}
}

void
CDataBuffer::getModelBones_2_8(
    char* buffer,
    const uintptr_t& size,
    std::vector<RigBone*>& bones,
    const std::vector<std::string>& stringTable)
{
    uint32_t numUnks0 = ReadUInt32(buffer);
    uint32_t numUnks1 = ReadUInt32(buffer);
    uint32_t numBones = ReadUInt32(buffer);
    uint32_t numUnks2 = ReadUInt32(buffer);

	std::vector<RigBone*> armature = bones;
	armature.resize(numBones);

	/* Iterate and collect all rig bones */
	for (int i = 0; i < numBones; i++){
		readBone2_8(buffer, stringTable, armature);
	}

	/* Filter irregular joints */
	for (auto& bone : armature) {
		if (bone)
			bones.push_back(bone);
	}
}

void
CDataBuffer::getModelBones_2_5(
    char* buffer,
    const uintptr_t & size,
    std::vector<RigBone*>&bones,
    const std::vector<std::string>&stringTable)
{
    uint32_t numUnks0 = ReadUInt32(buffer);
    uint32_t numBones = ReadUInt32(buffer);
    uint32_t numUnks1 = ReadUInt32(buffer);
    numBones = (size - 0xC) / 0x20;

	std::vector<RigBone*> armature = bones;
	armature.resize(numBones);

    /* Iterate and collect all rig bones */
    for (int i = 0; i < numBones; i++){
		readBone2_5(buffer, stringTable, armature);
    }

	/* Filter irregular joints */
	for (auto& bone : armature) {
		if (bone)
			bones.push_back(bone);
	}
}

void
CDataBuffer::getMaterials(char* buffer, const uintptr_t& size, std::vector<Material>& materials, const std::vector<std::string>& stringTable)
{
	uint32_t numMats = ReadUInt32(buffer);

	for (int i = 0; i < numMats; i++) {
		Material mat;
		uint32_t index = ReadUInt32(buffer);
		mat.name = stringTable.at(index);

		materials.push_back(mat);
	}
}


void setData(char* buffer, const MeshBuffer& mBuffer, Mesh& mesh)
{
	unsigned int hash = Data::hash(mBuffer.format.c_str());

	switch (hash)
	{
		case POSITION:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.vertices);
			break;
		case NORMALS:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.normals);
			break;
		case BINORMALS:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.binormals);
			break;
		case TANGENTS:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.tangents);
			break;
		case BLENDWEIGHTS:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.skin.weights);
			break;
		case BLENDINDICES:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.skin.indices);
			break;
		case TEXCOORDS:
			{
				UVMap uvChannel;
				Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, uvChannel.map);
				mesh.texcoords.push_back(uvChannel);
			}
			break;
		case COLOR:
			{
				VertexColorSet set;
				Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, set.map);
				mesh.colors.push_back(set);
			}
			break;
		default:
			break;
	}
}


void LoadMeshData(char*& buffer, Mesh& mesh, const std::vector<std::string>& strings)
{
	MeshBuffer mBuffer;
	mBuffer.format   = strings.at(ReadUInt16(buffer));
	mBuffer.property = strings.at(ReadUInt16(buffer));
	mBuffer.type	 = strings.at(ReadUInt32(buffer));
	buffer = Data::roundPointerToNearest4(buffer);

	/* Load mesh buffer charptr */
	uint32_t size = Data::getDataSetSize(mesh.numVerts, mBuffer.property);

	/* Collect and set mesh buffer data */
	setData(buffer, mBuffer, mesh);
	buffer += size;
}

void getSkinData(char*& buffer, Mesh& mesh)
{
	Skin& skin = mesh.skin;
	uint16_t jointEndIndex = ReadUInt16(buffer);
	skin.numWeights = ReadUInt16(buffer);
	if (skin.numWeights == 0) return;

	// iterate and collect indices and weights 
	for (int i = 0; i < mesh.numVerts; i++) {
		for (int j = 0; j < skin.numWeights; j++) {
			uint16_t index = ReadUInt16(buffer);
			skin.indices.push_back(index);
		}
	}
	buffer = Data::roundPointerToNearest4(buffer);

	for (int i = 0; i < mesh.numVerts; i++) {
		for (int j = 0; j < skin.numWeights; j++) {
			float weight = ( ReadUInt8(buffer) / 255.0 );
			skin.weights.push_back(weight);
		}
	}
	buffer = Data::roundPointerToNearest4(buffer);
}

void getVertexRemap(char*& buffer, Mesh& mesh) {
	std::vector<uint32_t> remapVector;

	for (int i = 0; i < mesh.numVerts; i++) {
		uint32_t index = ReadUInt32(buffer);
		remapVector.push_back(index);
	}
}


void
getTriangleBuffer(char*& buffer, Mesh* mesh, const std::vector<Material>& mtlTable, const std::vector<std::string>& strings) 
{
	std::vector<int> indices;
	uint16_t index = ReadUInt16(buffer);
	uint32_t numIndices = ReadUInt32(buffer);
	int numTriangles = numIndices / 3;
	
	/* Collect all face indices */
	for (int i = 0; i < numTriangles; i++) {
		Triangle face = (mesh->numVerts > 65535) ?
			Triangle{ ReadUInt32(buffer), ReadUInt32(buffer), ReadUInt32(buffer) } :
			Triangle{ ReadUInt16(buffer), ReadUInt16(buffer), ReadUInt16(buffer) };

		mesh->triangles.push_back(face);
	}

	/* Assign per face materials */
	buffer = Data::roundPointerToNearest4(buffer);
	uint32_t numMaterials = ReadUInt32(buffer);

	/* Collect all material face groups */
	for (int i = 0; i < numMaterials; i++) 
	{
		FaceGroup mtlGroup;
		uint32_t mtlIndex	  = ReadUInt32(buffer);
		mtlGroup.faceBegin	  = ReadUInt32(buffer) / 3;
		mtlGroup.numTriangles = ReadUInt32(buffer) / 3;
		mtlGroup.material     = mtlTable.at(mtlIndex);

		mesh->groups.push_back(mtlGroup);
	}

	buffer += 0x8;
}

void
CDataBuffer::getLods(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, 
	const std::vector<Material>& mtlTable, const std::vector<std::string>& strings)
{
	uint32_t numFaceBufs = ReadUInt32(buffer);
	uint32_t lodCap = (true) ? 1 : numFaceBufs;

	/* Iterate through face buffers and collect lods and material sets */
	for (int i = 0; i < lodCap; i++){
		uint32_t numIndices = ReadUInt32(buffer);
		for (int j = 0; j < numIndices; j++)
		{
			getTriangleBuffer( buffer, meshTable.at(j), mtlTable, strings );
		}
	}
}



static void loadColorMapInfo(char*& buffer, Mesh& mesh, const std::vector<std::string>& stringTable)
{
	for (auto& colorMap : mesh.colors) {
		int index = ReadUInt32(buffer);
		colorMap.name = stringTable.at(index);
	}
}

static void loadUVInfo(char*& buffer, Mesh& mesh, const std::vector<std::string>& stringTable)
{
	for (auto& map : mesh.texcoords) {
		int index = ReadUInt32(buffer);
		map.name = stringTable.at(index);
	}
}

static void getMeshMapInfo(char*& buffer, Mesh& mesh, const std::vector<std::string>& stringTable)
{
	/* def seems to always be "", has unknown use case */
	int index = ReadUInt32(buffer);
	mesh.definition = stringTable.at(index);
	assert(("Mesh has unknown def.", (mesh.definition == "")));

	/* detail map info */
	loadColorMapInfo(buffer, mesh, stringTable);
	loadUVInfo(buffer, mesh, stringTable);

	uint32_t bufferSig = 0;
	while (bufferSig != ENDM) {
		bufferSig = ReadUInt32(buffer);
		bufferSig = ntohl(bufferSig);
	}
	buffer += 0x4;
}

void getMorphWeights(char*& buffer, Mesh& mesh, const std::vector<std::string>& stringTable)
{
	/* Load all vertex morphs */
	vCMeshShapes blendshapes(buffer, stringTable, &mesh);
	blendshapes.load();
}

void getMeshData(char*& buffer, Mesh& mesh, const std::vector<std::string>& strings)
{
	uint32_t index, numStacks;
	index = ReadUInt32(buffer);

	mesh.sceneFlag = ReadUInt32(buffer);
	buffer += sizeof(uint16_t); // null const
	mesh.motionFlag = ReadUInt32(buffer);
	getAxisAlignedBoundingBox(buffer, mesh);

	mesh.numVerts = ReadUInt32(buffer);
	numStacks = ReadUInt32(buffer);
	mesh.name = strings.at(index);

	for (int j = 0; j < numStacks; j++) {
		uint32_t dataMagic = ReadUInt32(buffer);
		uint32_t typeMagic = ReadUInt32(buffer);
		uint32_t formatMagic = ReadUInt32(buffer);
		LoadMeshData(buffer, mesh, strings);
	}

	::getSkinData(buffer, mesh);
	::getVertexRemap(buffer, mesh);
	::getMorphWeights(buffer, mesh, strings);
	::getMeshMapInfo(buffer, mesh, strings);
}

void
CDataBuffer::getMeshes(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, const std::vector<std::string>& strings)
{
	uint32_t numModels = ReadUInt32(buffer);
	printf("\n\tTotal Models: %d\n", numModels);

	for (int i = 0; i < numModels; i++) {
		Mesh* mesh = new Mesh;

		getMeshData(buffer, *mesh, strings);
		meshTable.push_back(mesh);
		printf("\n\tRead Model: %s\n", mesh->name.c_str());
	}
}