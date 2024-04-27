#include <cmodelserializer.h>
#include <skinmodel.h>
#include <meshencoder.h>
#include <meshtags.h>
#include <BinaryIO.h>
#include "glm/gtx/euler_angles.hpp"
#include "winsock.h"

using namespace BinaryIO;

CModelSerializer::CModelSerializer(CSkinModel* target)
{
	this->m_model = target;
}

void CModelSerializer::save(const char* path) 
{
	m_savePath = path;
	this->serialize();
}

void CModelSerializer::serialize() 
{
	this->generateStringTable();
	this->createModelBuffer();
	this->createTextBuffer();
	this->createBoneBuffer();
	this->createMaterialBuffer();
	this->createMeshBufferDefs();
	this->createLODsBuffer();

	this->formatFile();
}

void CModelSerializer::createTextBuffer() 
{
	/* Initialize model buffer stream */
	StModelBf stream;
	uint32_t numStrings = m_stringTable.size();
	stream.type	 = "TEXT";
	stream.size  = MeshEncoder::getStringBufferSize(m_stringTable);
	stream.data  = new char[stream.size];

	char* tablePtr  = stream.data;
	char* stringPtr = (sizeof(uint32_t) + stream.data) + (sizeof(uint32_t) * (numStrings));

	/* Format string table */
	uint32_t offset = 0;
	WriteUInt32(tablePtr, numStrings);
	for (auto& string : m_stringTable) 
	{
		size_t size = string.size() + 1; // Include null terminator
		WriteUInt32(tablePtr, offset);
		memcpy(stringPtr, string.c_str(), size);

		offset	  += size;
		stringPtr += size;
	}

	::align_binary_stream(stringPtr);
	m_dataBuffers.push_back(stream);
}


inline 
void writeMatrixToBuffer(char*& buffer, const glm::mat4& matrix) 
{
	Vec3 rot;

	/* Decompose translation from bone matrix*/
	WriteFloat(buffer, matrix[3].x);
	WriteFloat(buffer, matrix[3].y);
	WriteFloat(buffer, matrix[3].z);

	/* Decompose euler angles from bone matix */
	glm::extractEulerAngleZYX(matrix, rot.z, rot.y, rot.x);
	WriteFloat(buffer, rot.x);
	WriteFloat(buffer, rot.y);
	WriteFloat(buffer, rot.z);
}

void CModelSerializer::createBoneBuffer()
{
	/* Initialize model buffer stream */
	const auto& bones = m_model->getBones();
	uint32_t numBones = bones.size();
	if (numBones == 0) return;

	StModelBf stream;
	stream.type = "BONE";
	stream.size = MeshEncoder::getBoneBufferSize(bones);
	stream.data = new char[stream.size];
	
	/* Write buffer table */
	char* buffer = stream.data;
	WriteUInt32(buffer, 0); // Unknown Data Enum
	WriteUInt32(buffer, 0); // Unknown Data Enum
	WriteUInt32(buffer, numBones); // Number of Bones
	WriteUInt32(buffer, 0); // Unknown Data Enum

	/* Write all bone data */
	for (auto& bone : bones) 
	{
		int16_t boneIndex   = indexOf(bone->name);
		int16_t parentIndex = (bone->parent) ? indexOf(bone->parent->name) : -1;

		/* Write bone index values*/
		WriteUInt16(buffer, boneIndex);
		WriteUInt16(buffer, parentIndex);
		writeMatrixToBuffer(buffer, bone->matrix_local);

		/* push new bone flags */
		WriteUInt8(buffer, 0);
		WriteUInt32(buffer, -1);
	}

	::align_binary_stream(buffer);
	m_dataBuffers.push_back(stream);
}


void CModelSerializer::createMaterialBuffer() 
{
	/* Initialize model buffer stream */
	const auto& meshes = m_model->getMeshes();
	uint32_t numMeshes = meshes.size();

	StModelBf stream;
	stream.type = "MTL!";
	stream.size = MeshEncoder::getMtlBufferSize(meshes);
	stream.data = new char[stream.size];
	char* buffer = stream.data;

	WriteUInt32(buffer, numMeshes);
	for (auto& mesh : meshes) {
		int32_t index = -1;

		if (mesh->groups.size() > 0) {
			FaceGroup& group = mesh->groups.front();
			index = indexOf(group.material.name);}

		WriteUInt32(buffer, index);
	}

	m_dataBuffers.push_back(stream);
}

void CModelSerializer::generateMeshBuffers(std::vector<StMeshBf>& buffers)
{
	const auto& meshes = m_model->getMeshes();

	for (auto& targetMesh : meshes) {
		StMeshBf meshbuffer;
		meshbuffer.mesh = targetMesh;

		serializeVertices(meshbuffer);
		serializeVertexNormals(meshbuffer);
		serializeTangents(meshbuffer);
		serializeBinormals(meshbuffer);
		serializeVertexColors(meshbuffer);
		serializeTexCoords(meshbuffer);
		serializeSkin(meshbuffer);
		serializeVertexRemap(meshbuffer);
		serializeBlendShapes(meshbuffer);
		serializeColorDict(meshbuffer);
		serializeUVDict(meshbuffer); // - 2k24 only

		buffers.push_back(meshbuffer);
	}
}

void CModelSerializer::writeBoundingBox(char*& buffer, const BoundingBox& box)
{
	WriteFloat(buffer, box.minX);
	WriteFloat(buffer, box.minY);
	WriteFloat(buffer, box.minZ);
	WriteFloat(buffer, box.maxX);
	WriteFloat(buffer, box.maxY);
	WriteFloat(buffer, box.maxZ);
}

static inline int getNumStacks(const StMeshBf& meshBuffer) {
	int numStacks = 0;

	meshBuffer.data.size();
	for (auto& stack : meshBuffer.data)
		if (stack->container != "COLOR_DICT" &&
			stack->container != "UV_DICT" &&
			stack->container != "VERTEX_REMAP" &&
			stack->container != "SKIN" &&
			stack->container != "BLENDSHAPES")
			numStacks++;

	return numStacks;
}

void CModelSerializer::writeMeshBuffer(char*& buffer, const StMeshBf& meshBuffer)
{
	auto& mesh = meshBuffer.mesh;
	WriteUInt32(buffer, indexOf(mesh->name));
	WriteUInt32(buffer, mesh->sceneFlag);
	WriteUInt16(buffer, 0); // alignment
	WriteUInt32(buffer, mesh->motionFlag);

	int numStacks = getNumStacks(meshBuffer);
	this->writeBoundingBox(buffer, mesh->bounds);
	WriteUInt32(buffer, mesh->numVerts);
	WriteUInt32(buffer, numStacks);

	/* Write data streams */
	for (auto& stack : meshBuffer.data) {
		std::string dataBf = stack->stream.str();
		WriteData(buffer, (char*)dataBf.c_str(), dataBf.size());
	}

	WriteUInt64(buffer, ntohl(ENDM));
}

void CModelSerializer::createMeshBufferDefs()
{	/* Collect all mesh buffer data */
	this->generateMeshBuffers(m_meshBuffers);

	StModelBf stream;
	stream.type = "MBfD";
	stream.size = MeshEncoder::getMeshBufferDefSize(m_meshBuffers);
	stream.data = new char[stream.size];

	char* buffer = stream.data;
	WriteUInt32(buffer, m_meshBuffers.size());

	for (auto& mshBf : m_meshBuffers) {
		writeMeshBuffer(buffer, mshBf);
	}

	/* Update and clean data stream */
	m_dataBuffers.push_back(stream);
	m_meshBuffers.clear();
}

void CModelSerializer::writeIndexBuffer(char*& buffer, int meshIndex) 
{
	auto mesh = m_model->getMeshes().at(meshIndex);
	uint32_t numTris = mesh->triangles.size();
	WriteUInt16(buffer, meshIndex);
	WriteUInt32(buffer, numTris * 3);

	bool use32bIndex = (mesh->numVerts > UINT16_MAX);
	for (auto& face : mesh->triangles) {
		(use32bIndex) ? WriteUInt32(buffer, face.x) : WriteUInt16(buffer, face.x);
		(use32bIndex) ? WriteUInt32(buffer, face.y) : WriteUInt16(buffer, face.y);
		(use32bIndex) ? WriteUInt32(buffer, face.z) : WriteUInt16(buffer, face.z);
	}

	::align_binary_stream(buffer);
}

void CModelSerializer::writeMaterialGroupBuffer(char*& buffer, int meshIndex)
{
	/* Write material groups */
	auto mesh = m_model->getMeshes().at(meshIndex);
	int numGroups = mesh->groups.size();
	WriteUInt32(buffer, numGroups);

	for (int i = 0; i < numGroups; i++) {
		auto& group = mesh->groups.at(i);
		WriteUInt32(buffer, meshIndex); // material index
		WriteUInt32(buffer, group.faceBegin);
		WriteUInt32(buffer, group.numTriangles * 3);
	}

	/* Write ENDM tag */
	WriteUInt64(buffer, ntohl(ENDM));
}

void CModelSerializer::createLODsBuffer() 
{
	StModelBf stream;
	const auto& meshes = m_model->getMeshes();
	uint32_t numLodLevels = 2;

	stream.type = "LODs";
	stream.size = MeshEncoder::getLodsBufferSize(meshes, numLodLevels);
	stream.data = new char[stream.size];
	char* buffer = stream.data;
	
	/* Write high/low LOD buffers */
	uint32_t numMeshes = meshes.size();
	WriteUInt32(buffer, numLodLevels);

	for (int i = 0; i < numLodLevels; i++) {
		WriteUInt32(buffer, numMeshes);

		for (int j = 0; j < numMeshes; j++) {
			writeIndexBuffer(buffer, j);
			writeMaterialGroupBuffer(buffer, j);
		}
	}

	m_dataBuffers.push_back(stream);
}

void CModelSerializer::createModelBuffer()
{
	StModelBf stream;

	stream.type = "MDL!";
	stream.size = MeshEncoder::getMDLBufferSize();
	stream.data = new char[stream.size];
	char* buffer = stream.data;

	WriteUInt32(buffer, 0x28); // File format version
	WriteUInt32(buffer, 0); // Unknown value
	writeBoundingBox(buffer, m_model->getAABBs()); // Model bounds

	m_dataBuffers.push_back(stream);
}

void CModelSerializer::writeDataBuffer(std::ofstream& fs, const StModelBf& modelBf) 
{
	fs.write(modelBf.type.c_str(), 0x4); // Stream Magic

	/* Write stream size info */
	if (modelBf.type != "MDL!")
		WriteUInt32(&fs, modelBf.size);

	/* Push stream binary to file stream */
	fs.write(modelBf.data, modelBf.size);
}

void CModelSerializer::formatFile()
{
	std::ofstream file(m_savePath, std::ios::binary);
	if (!file.is_open())
		return;

	/* Push all data streams to disk file */
	for (auto& dataBf : m_dataBuffers) {
		writeDataBuffer(file, dataBf);
		dataBf.free(); // Clean MDL data struct
	}

	WriteUInt64(&file, END); // Write END! tag
	file.close();
}


