#include "BinaryIO.h"
#include "meshtags.h"
#include "skinmodelpoly.h"
#include "winsock.h"
#include "modelfile.h"
using namespace BinaryIO;

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

void CSkinModel_2_0::readBone() // v2.0
{
	int16_t index = ReadInt16(m_data);
	int16_t parentIndex = ReadInt16(m_data);

	/* Get bone transformation matrix */
	RigBone* bone = loadBoneTransform(m_data);
	bone->index = index;
	bone->name = m_stringTable.at(index);

	/* Update bone hierarchy */
	m_bones.at(index) = bone;
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

void
CSkinModel_2_0::loadModelBones(const uintptr_t& size)
{
	uint32_t numBones = ReadUInt32(m_data);
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


void CSkinModel_2_0::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index);

	/* Load detail map info */
	loadColorMapInfo(mesh);
	::seekToEnd(m_data);
}

void CSkinModel_2_0::loadData()
{
	printf("Loading VCModel v%x\n", m_parent->getVersion());
	this->loadAxisBounds();
}

void CSkinModel_2_0::buildMesh(Mesh& mesh)
{
	uint32_t index, numStacks;
	index = ReadUInt32(m_data);

	mesh.sceneFlag = ReadUInt32(m_data);
	m_data += sizeof(uint16_t); // null const
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

	this->getVertexRemap(mesh);
	this->getMorphWeights(mesh);
	this->getMeshMapInfo(mesh);
}