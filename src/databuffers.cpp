#include "databuffers.h"
#include "BinaryIO.h"
#include "meshbuffers.h"
#include <meshtags.h>

using namespace BinaryIO;
using namespace VCModel;
using namespace MeshBuffers;


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

void 
CDataBuffer::getStringTable(char* buffer, const uintptr_t& blockPos, std::vector<std::string>& stringTable)
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
CDataBuffer::getModelBones_Ver_2_8(
    char* buffer,
    const uintptr_t& size,
    std::vector<RigBone*>& bones,
    const std::vector<std::string>& stringTable)
{

    uint32_t numUnks0, numUnks1, numUnks2, numBones;
    numUnks0 = ReadUInt32(buffer);
    numUnks1 = ReadUInt32(buffer);
    numBones = ReadUInt32(buffer);
    numUnks2 = ReadUInt32(buffer);

    printf("\n\tTotal Bones: %d", numBones);
    bones.resize(numBones);

    /* Iterate and collect all rig bones */
    for (int i = 0; i < numBones; i++)
    {
        if (i > 0xcf) {
            printf("");
        }
        int16_t index = ReadInt16(buffer);
        int16_t parentIndex = ReadInt16(buffer);
        bool isTypeJoint = !(index == 0 && parentIndex == 0);

        RigBone* bone = new RigBone;
        bone->translation = Vec3{ ReadFloat(buffer), ReadFloat(buffer), ReadFloat(buffer) };
        bone->quaternion = Vec4{ ReadFloat(buffer), ReadFloat(buffer),
                                  ReadFloat(buffer), 0.0 };

        int unkValueA = ReadUInt8(buffer);  /* Perhaps a flag? */
        int unkValueB = ReadUInt32(buffer); /* Unknown dword value */

        if (unkValueB != -1) {
            buffer += 0x10;
            buffer += 0x10;
        }

        /* Filter irregular joint types */
        if (isTypeJoint)
        {
            bone->name = stringTable.at(index);
            bones.at(index) = bone;
            if (parentIndex != -1) {
                bones.at(parentIndex)->children.push_back(bone);
                bone->parent = bones.at(parentIndex);
            }
        }
        else { delete bone; }
    }
}

void
CDataBuffer::getModelBones(
    char* buffer,
    const uintptr_t & size,
    std::vector<RigBone*>&bones,
    const std::vector<std::string>&stringTable,
    const int& containerType)
{

    switch (containerType)
    {
        case MDL_VERSION_2_8:
            CDataBuffer::getModelBones_Ver_2_8(buffer, size, bones, stringTable);
            return;
        default:
            break;
    }

    uint32_t numUnks0 = ReadUInt32(buffer);
    uint32_t numBones = ReadUInt32(buffer);
    uint32_t numUnks1 = ReadUInt32(buffer);

    printf("\n\tTotal Bones: %d", numBones);
    numBones = (size - 0xC) / 0x20;
    bones.resize(numBones);

    /* Iterate and collect all rig bones */
    for (int i = 0; i < numBones; i++)
    {
        int16_t index = ReadInt16(buffer);
        int16_t parentIndex = ReadInt16(buffer);
        bool isTypeJoint = !(index == 0 && parentIndex == 0);

        RigBone* bone = new RigBone;
        bone->translation = Vec3{ ReadFloat(buffer), ReadFloat(buffer), ReadFloat(buffer) };
        bone->quaternion  = Vec4{ ReadFloat(buffer), ReadFloat(buffer),
                                  ReadFloat(buffer), ReadFloat(buffer) };

        /* Filter irregular joint types */
        if (isTypeJoint)
        {
            bone->name = stringTable.at(index);
            bones.at(index) = bone;
            if (parentIndex != -1) {
                bones.at(parentIndex)->children.push_back(bone);
                bone->parent = bones.at(parentIndex);
            }
        }
        else { delete bone; }
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
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.skin.blendweights);
			break;
		case BLENDINDICES:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.skin.blendindices);
			break;
		case TEXCOORDS:
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, mesh.texcoords);
			break;
		case COLOR:
		{
			VertexColorSet set;
			Data::getDataSet(buffer, mesh.numVerts, mBuffer.type, mBuffer.property, set.data);
			mesh.colors.push_back(set);
		}
		break;
		default:
			break;
	}
}


void InitMeshBuffer(char*& buffer, Mesh& mesh, const std::vector<std::string>& strings)
{
	MeshBuffer mBuffer;
	mBuffer.format = strings.at(ReadUInt16(buffer));
	mBuffer.property = strings.at(ReadUInt16(buffer));
	mBuffer.type = strings.at(ReadUInt32(buffer));
	buffer = Data::roundPointerToNearest4(buffer);

	/* Load mesh buffer charptr */
	uint32_t size = Data::getDataSetSize(mesh.numVerts, mBuffer.property);
	char* meshBuffer = new char[size];
	memcpy(meshBuffer, buffer, size);

	/* Collect and set mesh buffer data */
	setData(buffer, mBuffer, mesh);
	buffer += size;
	delete[] meshBuffer;
}

void getSkinData(char*& buffer, Mesh& mesh) {

	Skin& skin = mesh.skin;
	uint16_t jointEndIndex = ReadUInt16(buffer);
	skin.numWeights = ReadUInt16(buffer);
	if (skin.numWeights == 0) return;

	// iterate and collect indices and weights 
	for (int i = 0; i < mesh.numVerts; i++) {
		for (int j = 0; j < skin.numWeights; j++) {
			uint16_t index = ReadUInt16(buffer);
			skin.blendindices.push_back(index);
		}
	}

	for (int i = 0; i < mesh.numVerts; i++) {
		for (int j = 0; j < skin.numWeights; j++) {
			float weight = ( ReadUInt8(buffer) / 255.0 );
			skin.blendweights.push_back(weight);
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

void getMorphWeights(char*& buffer, Mesh& mesh) {
	uint32_t numMorphs = ReadUInt32(buffer);
	uint32_t bufferSig = 0;

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
		InitMeshBuffer(buffer, mesh, strings);
	}

	getSkinData(buffer, mesh);
	getVertexRemap(buffer, mesh);
	getMorphWeights(buffer, mesh);
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
	for (int i = 0; i < numMaterials; i++) {
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
