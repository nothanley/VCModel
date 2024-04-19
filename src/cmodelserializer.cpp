#include <cmodelserializer.h>
#include <skinmodel.h>
#include <meshencoder.h>
#include <meshtags.h>
#include <BinaryIO.h>
#include "glm/gtx/euler_angles.hpp"

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
	this->createTextBuffer();
	this->createBoneBuffer();
	this->createMaterialBuffer();
	this->createMeshBufferDefs();
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
	WriteUInt32_CharStream(tablePtr, numStrings);
	for (auto& string : m_stringTable) 
	{
		size_t size = string.size() + 1; // Include null terminator
		WriteUInt32_CharStream(tablePtr, offset);
		memcpy(stringPtr, string.c_str(), size);

		offset	  += size;
		stringPtr += size;
	}

	m_dataBuffers.push_back(stream);
}


inline 
void writeMatrixToBuffer(char*& buffer, const glm::mat4& matrix) 
{
	float rotX, rotY, rotZ;

	/* Decompose translation from bone matrix*/
	WriteFloat_CharStream(buffer, matrix[3][2]);
	WriteFloat_CharStream(buffer, matrix[3][1]);
	WriteFloat_CharStream(buffer, matrix[3][0]);

	/* Decompose euler angles from bone matix */
	glm::extractEulerAngleXYZ(matrix, rotX, rotY, rotZ);
	WriteFloat_CharStream(buffer, rotX);
	WriteFloat_CharStream(buffer, rotY);
	WriteFloat_CharStream(buffer, rotZ);
}

void CModelSerializer::createBoneBuffer()  // debug format is mdl v2.8
{
	/* Initialize model buffer stream */
	const auto& bones = m_model->getBones();
	uint32_t numBones = bones.size();

	StModelBf stream;
	stream.type = "BONE";
	stream.size = MeshEncoder::getBoneBufferSize(bones);
	stream.data = new char[stream.size];
	
	/* Write buffer table */
	char* buffer = stream.data;
	WriteUInt32_CharStream(buffer, 0); // Unknown Data Enum
	WriteUInt32_CharStream(buffer, 0); // Unknown Data Enum
	WriteUInt32_CharStream(buffer, numBones); // Number of Bones
	WriteUInt32_CharStream(buffer, 0); // Unknown Data Enum

	/* Write all bone data */
	for (auto& bone : bones) 
	{
		int16_t boneIndex   = indexOf(bone->name);
		int16_t parentIndex = (bone->parent) ? indexOf(bone->parent->name) : -1;

		/* Write bone index values*/
		WriteUInt16_CharStream(buffer, boneIndex);
		WriteUInt16_CharStream(buffer, parentIndex);
		writeMatrixToBuffer(buffer, bone->matrix);

		/* push new bone flags */
		WriteUInt8_CharStream(buffer, 0);
		WriteUInt32_CharStream(buffer, -1);
	}

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

	WriteUInt32_CharStream(buffer, numMeshes);
	for (auto& mesh : meshes) {
		int32_t index = -1;

		if (mesh->groups.size() > 0) {
			FaceGroup& group = mesh->groups.front();
			index = indexOf(group.material.name);}

		WriteUInt32_CharStream(buffer, index);
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

		buffers.push_back(meshbuffer);
	}
}

void CModelSerializer::writeBoundingBox(char*& buffer, Mesh* mesh) 
{
	BoundingBox& box = mesh->bounds;
	WriteFloat_CharStream(buffer, box.minX);
	WriteFloat_CharStream(buffer, box.minY);
	WriteFloat_CharStream(buffer, box.minZ);

	WriteFloat_CharStream(buffer, box.maxX);
	WriteFloat_CharStream(buffer, box.maxY);
	WriteFloat_CharStream(buffer, box.maxZ);
}

void CModelSerializer::writeMeshBuffer(char*& buffer, const StMeshBf& meshBuffer) {
	auto& mesh = meshBuffer.mesh;
	WriteUInt32_CharStream(buffer, indexOf(mesh->name));
	WriteUInt32_CharStream(buffer, mesh->sceneFlag);
	WriteUInt16_CharStream(buffer, mesh->motionFlag);

	this->writeBoundingBox(buffer, mesh);
	WriteUInt32_CharStream(buffer, mesh->numVerts);
	WriteUInt32_CharStream(buffer, meshBuffer.data.size());

	/* Write data streams */
	for (auto& stack : meshBuffer.data) {
		std::string dataBf = stack->stream.str();
		WriteData(buffer, (char*)dataBf.c_str(), dataBf.size());
	}

	WriteUInt32_CharStream(buffer, ENDM);
	WriteUInt32_CharStream(buffer, 0x0);
}

void CModelSerializer::createMeshBufferDefs()
{
	/* Collect all mesh buffer data */
	this->generateMeshBuffers(m_meshBuffers);

	StModelBf stream;
	stream.type = "MBfD";
	stream.size = MeshEncoder::getMeshBufferDefSize(m_meshBuffers);
	stream.data = new char[stream.size];

	char* buffer = stream.data;
	WriteUInt32_CharStream(buffer, m_meshBuffers.size());

	for (auto& mshBf : m_meshBuffers) {
		writeMeshBuffer(buffer, mshBf);
	}

	/* Update and clean data stream */
	m_dataBuffers.push_back(stream);
	m_meshBuffers.clear();
}

