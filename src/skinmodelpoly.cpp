#include "BinaryIO.h"
#include "meshtags.h"
#include "skinmodelpoly.h"
#include "winsock.h"
using namespace BinaryIO;

void 
CSkinModel_2_8::loadBuffer()
{
	uint32_t type = ReadUInt32(m_data);
	uint32_t size = ReadUInt32(m_data);
	char* nextBfPtr = m_data + size;

	switch (type) // Handle all streams
	{
		case TEXT:
			this->loadStringTable();
			break;
		case BONE:
			this->loadModelBones(size);
			break;
		case MTL:
			this->loadMaterials();
			break;
		case MBfD:
			this->loadMeshes();
			break;
		case LODs:
			this->loadLods();
			break;
		case END:
			return;
		default:
			break;
	}

	//Iterate through model structure
	m_data = nextBfPtr;
	loadBuffer();
}

void
CSkinModel_2_5::loadBuffer()
{
	uint32_t type = ReadUInt32(m_data);
	uint32_t size = ReadUInt32(m_data);
	char* nextBfPtr = m_data + size;

	// Handle stream
	switch (type)
	{
		case TEXT:
			this->loadStringTable();
			break;
		case BONE:
			this->loadModelBones(size);
			break;
		case MTL:
			this->loadMaterials();
			break;
		case MBfD:
			this->loadMeshes();
			break;
		case LODs:
			this->loadLods();
			break;
		case END:
			return;
		default:
			break;
	}

	//Iterate through model structure
	m_data = nextBfPtr;
	loadBuffer();
}


void CSkinModel_2_8::readBone() // v2.8
{
	int16_t index = ReadInt16(m_data);
	int16_t parentIndex = ReadInt16(m_data);
	bool isTypeJoint = !(index == 0 && parentIndex == 0);

	/* Get bone transformation matrix */
	RigBone* bone = loadBoneTransform(m_data);
	bone->index = index;
	int8_t  unkValueA = ReadUInt8(m_data);  /* Perhaps a flag? */
	int32_t unkValueB = ReadUInt32(m_data); /* Unknown dword value */

	if (unkValueB != -1)
		m_data += 0x20;

	/* Filter irregular joint types */
	if (!isTypeJoint) {
		delete bone;
		return;
	}

	m_bones.at(index) = (isTypeJoint) ? bone : nullptr;
	bone->name = m_stringTable.at(index);

	/* Update bone hierarchy */
	if (parentIndex != -1)
		bone->set_parent(m_bones.at(parentIndex));
}

void CSkinModel_2_5::readBone() // v2.5
{
	int16_t index = ReadInt16(m_data);
	int16_t parentIndex = ReadInt16(m_data);
	bool isTypeJoint = !(index == 0 && parentIndex == 0);

	/* Get bone transformation matrix */
	RigBone* bone = loadBoneTransform(m_data);
	bone->index = index;
	m_data += sizeof(uint32_t);

	/* Filter irregular joint types */
	if (!isTypeJoint) {
		delete bone;
		return;
	}

	m_bones.at(index) = (isTypeJoint) ? bone : nullptr;
	bone->name = m_stringTable.at(index);

	/* Update bone hierarchy */
	if (parentIndex != -1)
		bone->set_parent(m_bones.at(parentIndex));
}

void
CSkinModel_2_8::loadModelBones(const uintptr_t& size) // v2.8
{
	uint32_t numUnks0 = ReadUInt32(m_data);
	uint32_t numUnks1 = ReadUInt32(m_data);
	uint32_t numBones = ReadUInt32(m_data);
	uint32_t numUnks2 = ReadUInt32(m_data);
	m_bones.resize(numBones);

	/* Iterate and collect all rig bones */
	for (int i = 0; i < numBones; i++) {
		this->readBone();
	}

	/* Filter irregular joints */
	std::vector<RigBone*> filtered_bones;
	for (auto& bone : m_bones) {
		if (bone) 
			filtered_bones.push_back(bone);
	}
	m_bones = filtered_bones;
}

void
CSkinModel_2_5::loadModelBones(const uintptr_t& size)
{
	uint32_t numUnks0 = ReadUInt32(m_data);
	uint32_t numBones = ReadUInt32(m_data);
	uint32_t numUnks1 = ReadUInt32(m_data);
	numBones = (size - 0xC) / 0x20;
	m_bones.resize(numBones);

	/* Iterate and collect all rig bones */
	for (int i = 0; i < numBones; i++) {
		this->readBone();
	}

	/* Filter irregular joints */
	std::vector<RigBone*> filtered_bones;
	for (auto& bone : m_bones) {
		if (bone)
			filtered_bones.push_back(bone);
	}
	m_bones = filtered_bones;
}

static inline void seekToEnd(char*& buffer) 
{
	uint32_t bufferSig = 0;
	while (bufferSig != ENDM) {
		bufferSig = ReadUInt32(buffer);
		bufferSig = ntohl(bufferSig);
	}
	buffer += 0x4;
}

void CSkinModel_2_8::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index); // Def seems to always be "", has unknown use case

	/* Load detail map info */
	loadColorMapInfo(mesh);
	loadUVInfo(mesh);
	::seekToEnd(m_data);
}

void CSkinModel_2_5::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index);

	/* Load detail map info */
	loadColorMapInfo(mesh);
	::seekToEnd(m_data);
}