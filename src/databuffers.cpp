#include "databuffers.h"
#include "BinaryIO.h"
#include "meshbuffers.h"
#include <meshtags.h>
#include "winsock.h"
#include "../meshshapes.h"
#include <glm/gtx/euler_angles.hpp>

using namespace BinaryIO;
using namespace MeshSerializer;

inline 
glm::mat4 dot_4x4(const glm::mat4x4& a, const glm::mat4x4& b)
{
	glm::mat4 result;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			float num = 0;

			for (int k = 0; k < 4; k++)
				num += a[i][k] * b[k][j];

			result[i][j] = num;
		}

	return result;
}

inline 
void mapBoneToParentSpace(RigBone* child, const RigBone* parent)
{
	child->matrix = dot_4x4(child->matrix, parent->matrix);
}

inline 
RigBone* loadBoneTransform(char*& buffer)
{
	/* Reads bone transformation matrix */
	RigBone* bone = new RigBone;
	glm::vec3 position( ReadFloat(buffer), ReadFloat(buffer), ReadFloat(buffer) );
	glm::vec3 rotation( ReadFloat(buffer), ReadFloat(buffer), ReadFloat(buffer) );

	bone->matrix     = glm::eulerAngleZYX( rotation.x, rotation.y, rotation.z );
	bone->matrix[3]  = glm::vec4(          position.z, position.y, position.x, 1.0f);
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
	bones.at(index)   = (isTypeJoint) ? bone : nullptr;
	int8_t  unkValueA = ReadUInt8(buffer);  /* Perhaps a flag? */
	int32_t unkValueB = ReadUInt32(buffer); /* Unknown dword value */

	if (unkValueB != -1)
		buffer += 0x20;

	/* Filter irregular joint types */
	if (!isTypeJoint) {
		delete bone;
		return; }
	
	bone->name = stringTable.at(index);

	/* Update bone transform to world-space*/
	if (parentIndex == -1) return;
	bones.at(parentIndex)->children.push_back(bone);
	bone->parent = bones.at(parentIndex);
	mapBoneToParentSpace(bone, bone->parent);
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
	bones.at(index) = (isTypeJoint) ? bone : nullptr;
	buffer += sizeof(uint32_t);

	/* Filter irregular joint types */
	if (!isTypeJoint) {
		delete bone;
		return; }

	bone->name = stringTable.at(index);

	/* Update bone transform to world-space*/
	if (parentIndex == -1) return;
	bones.at(parentIndex)->children.push_back(bone);
	bone->parent = bones.at(parentIndex);
	mapBoneToParentSpace(bone, bone->parent);
}



void getAxisAlignedBoundingBox(char*& buffer, Mesh& mesh, bool getRadius=true) {
	BoundingBox& box = mesh.bounds;
	if (getRadius)
		box.radius = ReadFloat(buffer);

	box.minX = ReadFloat(buffer);
	box.minY = ReadFloat(buffer);
	box.minZ = ReadFloat(buffer);
	box.maxX = ReadFloat(buffer);
	box.maxY = ReadFloat(buffer);
	box.maxZ = ReadFloat(buffer);
}

void Mesh::flipNormals()
{
	for (auto& tri : triangles)
		tri = Triangle{ tri.y, tri.x, tri.z };
}

void Mesh::convertSplitNorms()
{
	std::vector<float> data;
	for (int i = 0; i < numVerts; i++)
	{
		int index = (i * 4);
		data.push_back(normals.at(index + 0));
		data.push_back(normals.at(index + 1));
		data.push_back(normals.at(index + 2));
	}

	this->normals = data;
}

void Mesh::translateUVs(const int& index)
{
	if (index > texcoords.size())
		return;

	auto& map = this->texcoords.at(index).map;
	/* Flip Y axis and translate up by one unit */
	for (int i = 0; i < map.size(); i += 2) {
		map[i + 1] = -(map[i + 1] - 1.0f);
	}
}

inline void 
LoadVertexSkin(const Skin* skin, BlendWeight& skinVertex, 
	const std::vector<std::string>& stringTable, int& begin, const int& numWeights)
{
	/* Iterate through all specified weights for current vertex */
	for (int j = 0; j < numWeights; j++)
	{
		int index = skin->indices[begin];
		float influence = skin->weights[begin];

		std::string boneName = stringTable.at(index);
		skinVertex.bones.push_back(boneName);
		skinVertex.weights.push_back(influence);

		begin++; // Update skin pointer to next index
	}
}

std::vector<BlendWeight>* 
Skin::unpack(const std::vector<std::string>& stringTable)
{
	int numVerts = this->weights.size() / numWeights;
	std::vector<BlendWeight>* skinData = new std::vector<BlendWeight>;
	skinData->resize(numVerts);

	/* Iterate through all skin vertices */
	for (int i = 0; i < numVerts; i++)
	{
		BlendWeight& skinVtx = skinData->at(i);
		int skinPtr = (i * numWeights);
		LoadVertexSkin(this, skinVtx, stringTable, skinPtr, numWeights);
	}

	return skinData;
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
    bones.resize(numBones);

	/* Iterate and collect all rig bones */
	for (int i = 0; i < numBones; i++){
		readBone2_8(buffer, stringTable, bones);
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
    bones.resize(numBones);

    /* Iterate and collect all rig bones */
    for (int i = 0; i < numBones; i++){
		readBone2_5(buffer, stringTable, bones);
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

void getMorphWeights(char*& buffer, Mesh& mesh, const std::vector<std::string>& stringTable) {
	uint32_t bufferSig = 0;

	/* Load all object morphs */
	vCMeshShapes blendshapes(buffer, stringTable, &mesh);
	blendshapes.load();

	while (bufferSig != ENDM) {
		bufferSig = ReadUInt32(buffer);
		bufferSig = ntohl(bufferSig);
	}

	buffer += 0x4;
}

void getMeshData(char*& buffer, Mesh& mesh, const std::vector<std::string>& strings) 
{
	uint32_t index, numStacks;
	index = ReadUInt32(buffer);

	mesh.sceneFlag = ReadUInt32(buffer);
	mesh.motionFlag = ReadInt16(buffer);
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

	getSkinData(buffer, mesh);
	getVertexRemap(buffer, mesh);
	getMorphWeights(buffer, mesh, strings);
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
		printf( "\n\tRead Model: %s\n" , mesh->name.c_str() );
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

	/* Ignore face settings and assign last selected material to mesh*/
	for (int i = 0; i < numMaterials; i++) 
	{
		uint32_t matIndex = ReadUInt32(buffer);
		uint32_t affectedFaceIndexFirst = ReadUInt32(buffer);
		uint32_t affectedFaceIndexSize = ReadUInt32(buffer);
		mesh->material = mtlTable.at(matIndex);
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
	for (int i = 0; i < lodCap; i++)
	{
		uint32_t numLods = ReadUInt32(buffer);

		for (int j = 0; j < numLods; j++) 
		{
			getTriangleBuffer( buffer, meshTable.at(j), mtlTable, strings );
		}
	}

}
