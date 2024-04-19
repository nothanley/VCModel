#include "CModelSerializer.h"
#include <skinmodel.h>
#include <cmeshserializer.h>
#include <meshencoder.h>
#include <BinaryIO.h>
#include "glm/gtx/euler_angles.hpp"
#include <crc32c/crc32c.h>

using namespace BinaryIO;
using namespace crc32c;


const std::vector<std::string> STREAM_TABLE{
	"POSITION", "R32_G32_B32","float", "NORMAL", "R8_G8_B8_A8", "snorm",
	"TANGENT","BINORMAL","R8", "COLOR","unorm","TEXCOORD",
	"R32_G32","R16_G16_B16_A16","BLENDINDICES", "uint","BLENDWEIGHTS","R32_G32_B32_A32",""
};

CModelSerializer::CModelSerializer(CSkinModel* target) :
	m_model(target)
{}

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


inline int get_str_index(const std::vector<std::string>& table, const std::string& target)
{
	int index = -1;
	int numStrings = table.size();

	for (int i = 0; i < numStrings; i++) {
		const std::string& element = table.at(i);
		if (target == element)
			return i;
	}

	return -1;
}

int CModelSerializer::indexOf(const std::string& target){
	return get_str_index(m_stringTable, target);
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

void StDataBf::setHeader(const std::vector<std::string>& table, const char* data, const char* format, const char* type)
{   
	/* Define stream container */
	this->container = data;
	WriteUInt32(stream, ::crc32c_lower(data));
	WriteUInt32(stream, ::crc32c_lower(format));
	WriteUInt32(stream, ::crc32c_lower(type));

	WriteUInt16( stream, ::get_str_index(table, data));
	WriteUInt16( stream, ::get_str_index(table, format));
	WriteUInt16( stream, ::get_str_index(table, type));
	::align_binary_stream(stream);
}

void CModelSerializer::createMeshBufferDefs()
{
	/* Collect all mesh buffer data */
	CMeshSerializer serializer(this);
	serializer.generateMeshBuffers(m_meshBuffers);

	/* Merge stream data... */
	uint32_t size = MeshEncoder::getMeshBufferDefSize(m_meshBuffers);

	/* ... */


	/* Clear all data streams */
	m_meshBuffers.clear();
}


void CModelSerializer::generateStringTable()
{
	/* Push all bone names */
	auto bones = m_model->getBones();
	for (auto& bone : bones) {
		if (bone) {
			m_stringTable.push_back(bone->name);
		}
	}

	/* Push all mesh+material names */
	auto meshes = m_model->getMeshes();
	for (auto& mesh : meshes) {
		m_stringTable.push_back(mesh->name);

		for (auto& group : mesh->groups)
			m_stringTable.push_back(group.material.name);
	}

	/* Push all blendshape ids */
	for (auto& mesh : meshes)
		for (auto& shape : mesh->blendshapes) {
			m_stringTable.push_back(shape.name);
		}

	/* Push all stream types */
	for (auto& type : STREAM_TABLE) {
		m_stringTable.push_back(type);
	}
}




