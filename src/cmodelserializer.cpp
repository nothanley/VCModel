#include "CModelSerializer.h"
#include <skinmodel.h>
#include <BinaryIO.h>
#include <meshbuffers.h>
#include "glm/gtx/euler_angles.hpp"
#include <crc32c/crc32c.h>
#include <algorithm>

using namespace BinaryIO;
using namespace MeshSerializer;
using namespace crc32c;

CModelSerializer::CModelSerializer(CSkinModel* target) :
	m_model(target)
{
	this->buildStringTable();
}

void CModelSerializer::save(const char* path) 
{
	m_savePath = path;
	this->serialize();
}

void CModelSerializer::serialize() 
{
	this->createTextBuffer();
	this->createBoneBuffer();
	this->createMaterialBuffer();
	this->createMeshBufferDefs();
	// ...
}

inline void align_binary_stream(std::stringstream& stream) {
	while (stream.tellp() % 4 != 0) {
		WriteByte(stream, 0);
	}
}

inline uint32_t CModelSerializer::getBoneBufferSize(const std::vector<RigBone*>& bones)
{
	uint32_t tableLength = sizeof(uint32_t) * 4;     // Varies with revision type
	uint32_t entrySize = sizeof(uint16_t) * 2;		 // index + parent
	entrySize += sizeof(uint32_t) * 24;				 // translate + rotation vectors
	entrySize += sizeof(uint8_t) + sizeof(uint32_t); // Unknown values - new
	entrySize *= bones.size();

	return tableLength + entrySize;
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

inline uint32_t crc32c_lower(std::string string) {
	std::transform(string.begin(), string.end(), string.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return Crc32c(string);
}

inline 
uint32_t CModelSerializer::getStringBufferSize(const std::vector<std::string>& strings) {
	uint32_t size = sizeof(uint32_t);
	size += (strings.size() * sizeof(uint32_t));

	for (auto& string : strings) {
		size += (string.size() + 1); }
	return size;
}

void CModelSerializer::createTextBuffer() 
{
	/* Initialize model buffer stream */
	StModelBf stream;
	uint32_t numStrings = m_stringTable.size();
	stream.type	 = "TEXT";
	stream.size  = getStringBufferSize(m_stringTable);
	stream.data  = new char[stream.size];

	char* tablePtr  = stream.data;
	char* stringPtr = (sizeof(uint32_t) + stream.data) + (sizeof(uint32_t) * (numStrings));

	/* Format string table */
	uint32_t offset = 0;
	WriteUInt32_CharStream(tablePtr, numStrings);
	for (auto& string : m_stringTable) 
	{
		WriteUInt32_CharStream(tablePtr, offset);
		size_t size = string.size() + 1; // Include null terminator
		memcpy(stringPtr, string.c_str(), size);

		offset	  += size;
		stringPtr += size;
	}

	m_dataBuffers.push_back(stream);
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
	stream.size = getBoneBufferSize(bones);
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

inline 
uint32_t CModelSerializer::getMtlBufferSize(const std::vector<Mesh*>& meshes)
{
	uint32_t size = sizeof(uint32_t);
	size += (sizeof(uint32_t) * meshes.size());
	return size;
}

void CModelSerializer::createMaterialBuffer() 
{
	/* Initialize model buffer stream */
	const auto& meshes = m_model->getMeshes();
	uint32_t numMeshes = meshes.size();

	StModelBf stream;
	stream.type = "MTL!";
	stream.size = getMtlBufferSize(meshes);
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

inline
uint32_t CModelSerializer::getMeshBufferDefSize(const std::vector<Mesh*>& meshes)
{
	uint32_t size = sizeof(uint32_t); // Num Meshes
	uint32_t numMeshes = meshes.size();

	for (int i = 0; i < numMeshes; i++)
	{
		auto& mesh	 = meshes.at(i);
		size += sizeof(uint32_t) * 3; // index + flags
		size += sizeof(uint16_t);	  // Null
		size += sizeof(uint32_t) * 6; // mesh AABBs
		size += sizeof(uint32_t) * 2; // numverts + blocks

		//for (auto& stream : defs)
			//size += stream.size();
	}

	return size;
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

void CModelSerializer::serializeVertices(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "POSITION", "R32_G32_B32", "float");

	/* Write vertex buffer */
	std::vector<float>& vertices = target.mesh->vertices;
	dataBf->stream.write((char*)vertices.data(), vertices.size());
	::align_binary_stream(dataBf->stream);

	target.buffers.push_back(dataBf);
}

void CModelSerializer::serializeVertexNormals(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "NORMAL", "R8_G8_B8_A8", "snorm");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& normals = target.mesh->normals;
	int numNormals = normals.size() / 3;

	for (int i = 0; i < numNormals; i+=3) {
		// Convert to signed normals
		Vec3 normal{ normals[i], normals[i + 1], normals[i + 2] };
		normal.pack_values(1.0);
		normal *= 127;

		WriteSInt8(stream, normal.x);
		WriteSInt8(stream, normal.z);
		WriteSInt8(stream, normal.y);
		WriteSInt8(stream, 0);
	}

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}


void CModelSerializer::createMeshBufferDefs()
{
	/* Collect all mesh buffer data */
	const auto& meshes = m_model->getMeshes();
	for (auto& targetMesh : meshes)
	{
		StMeshBf meshbuffer;
		meshbuffer.mesh = targetMesh;

		/* Serialize all child data streams*/
		serializeVertices(meshbuffer);
		serializeVertexNormals(meshbuffer);

		/* ... */
		m_meshBuffers.push_back(meshbuffer);
	}


	/* Merge stream data... */
	/* ... */

	m_meshBuffers.clear();
}


void CModelSerializer::buildStringTable()
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

