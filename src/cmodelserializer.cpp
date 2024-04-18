#include "CModelSerializer.h"
#include <skinmodel.h>
#include "BinaryIO.h"
using namespace BinaryIO;

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
	// ...
}

inline uint32_t getStringBufferSize(const std::vector<std::string>& strings) {
	uint32_t size = sizeof(uint32_t);
	size += (strings.size() * sizeof(uint32_t));

	for (auto& string : strings) {
		size += string.size();
		size++;
	}
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

inline uint32_t getBoneBufferSize(const std::vector<RigBone*>& bones) {

}

void CModelSerializer::createBoneBuffer() 
{
	/* Initialize model buffer stream */
	const auto& bones = m_model->getBones();
	uint32_t numBones = bones.size();

	StModelBf stream;
	stream.type = "BONE";
	stream.size = getBoneBufferSize(bones);
	stream.data = new char[stream.size];

	/* ... */
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
		m_stringTable.push_back(mesh->material.name);

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

