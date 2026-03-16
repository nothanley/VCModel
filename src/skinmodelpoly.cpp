#include "MemoryReader/memoryreader.h"
#include "meshtags.h"
#include "skinmodelpoly.h"
#include "winsock.h"
#include "modelfile.h"
#include "blendshapes_legacy.h"
#include "meshbuffers.h"

using namespace memreader;

void CSkinModel_2_15::readBone() // v2.15
{
	int16_t index = ReadInt16(m_data);
	int16_t parentIndex = ReadInt16(m_data);

	/* Check valid bone indices */
	if (index == 0 && parentIndex == 0)
		throw std::runtime_error("Failed to load bone data.");

	/* Get bone transformation matrix */
	RigBone* bone = loadBoneTransform(m_data);
	int8_t  use_jig = ReadUInt8(m_data);
	int32_t jIndex = ReadUInt32(m_data);
	bone->index = index;
	bone->name = m_stringTable.at(index);
	m_bones[index] = bone;

	/* Load special bone jiggle info */
	if (jIndex >= 0)
	{
		auto boneId = m_stringTable.at(bone->index);
		auto jig = std::make_shared<JigParam>();
		jig->index = jIndex;
		jig->unk1 = ReadUInt32(m_data);
		jig->unk2 = ReadUInt32(m_data);
		jig->weightsA = { ReadFloat(m_data), ReadFloat(m_data), ReadFloat(m_data) };
		jig->weightsB = { ReadFloat(m_data), ReadFloat(m_data), ReadFloat(m_data) };
		bone->jig = (jig->unk1 == NULL || jig->unk2 == NULL) ? jig : NULL;
	}

	/* Update bone hierarchy */
	if (parentIndex != -1)
		bone->set_parent(m_bones.at(parentIndex));
}

void CSkinModel_2_9::readBone() // v2.9
{
	int16_t index       = ReadInt16(m_data);
	int16_t parentIndex = ReadInt16(m_data);

	/* Check valid bone indices */
	if (index == 0 && parentIndex == 0)
		throw std::runtime_error("Failed to load bone data.");

	/* Get bone transformation matrix */
	RigBone* bone   = loadBoneTransform(m_data);
	int8_t  use_jig = ReadUInt8(m_data);
	int32_t jIndex  = ReadUInt32(m_data);
	bone->index    = index;
	bone->name     = m_stringTable.at(index);
	m_bones[index] = bone;

	/* Load special bone jiggle info */
	if (jIndex >= 0)
	{
		auto boneId     = m_stringTable.at(bone->index);
		auto jig        = std::make_shared<JigParam>();
		jig->index      = jIndex;
		jig->unk1       = ReadUInt32(m_data);
		jig->unk2       = ReadUInt32(m_data);
		jig->weightsA   = { ReadFloat(m_data), ReadFloat(m_data), ReadFloat(m_data) };
		jig->weightsB   = { ReadFloat(m_data), ReadFloat(m_data), ReadFloat(m_data) };
		bone->jig       = (jig->unk1 == NULL || jig->unk2 == NULL) ? jig : NULL;
	}

	/* Update bone hierarchy */
	if (parentIndex != -1)
		bone->set_parent(m_bones.at(parentIndex));
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
CSkinModel_2_15::loadModelBones(const uintptr_t& size) // v2.15
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
CSkinModel_2_9::loadModelBones(const uintptr_t& size) // v2.9
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

void CSkinModel_2_15::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index); // Def seems to always be "", has unknown use case

	/* Load detail map info */
	loadColorMapInfo(mesh);
	loadUVInfo(mesh);
	seekToEnd(m_data);
}

void CSkinModel_2_9::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index); // Def seems to always be "", has unknown use case

	/* Load detail map info */
	loadColorMapInfo(mesh);
	loadUVInfo(mesh);
	seekToEnd(m_data);
}

void CSkinModel_2_8::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index); // Def seems to always be "", has unknown use case

	/* Load detail map info */
	loadColorMapInfo(mesh);
	loadUVInfo(mesh);
	seekToEnd(m_data);
}

void CSkinModel_2_5::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index);

	/* Load detail map info */
	loadColorMapInfo(mesh);
	seekToEnd(m_data);
}


void CSkinModel_2_0::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index);

	/* Load detail map info */
	loadColorMapInfo(mesh);
	seekToEnd(m_data);
}

void CSkinModel_2_0::loadData()
{
	//printf("Loading VCModel v%x\n", m_parent->getVersion());
	this->loadAxisBounds();
}

void CSkinModel_1_1::loadData()
{
	//printf("Loading VCModel v%x\n", m_parent->getVersion());
	this->loadAxisBounds();
}


void CSkinModel_2_15::buildMesh(Mesh& mesh)
{
	uint32_t index, numStacks;
	index = ReadUInt32(m_data);

	mesh.sceneFlag = ReadUInt32(m_data);
	m_data += sizeof(uint32_t); // null const
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

	/* Ignore data stream if using lightweight loader */
	if (m_parent->getLoadType() == enModelDefs::LoadLightWeight) {
		seekToEnd(m_data);
		return;
	}

	this->getSkinData(mesh);
	this->getVertexRemap(mesh);
	this->getMorphWeights(mesh);
	this->getMeshMapInfo(mesh);
}

void CSkinModel_2_9::buildMesh(Mesh& mesh)
{
	uint32_t index, numStacks;
	index = ReadUInt32(m_data);

	mesh.sceneFlag = ReadUInt32(m_data);
	m_data += sizeof(uint32_t); // null const
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

	/* Ignore data stream if using lightweight loader */
	if (m_parent->getLoadType() == enModelDefs::LoadLightWeight) {
		seekToEnd(m_data);
		return;
	}

	this->getSkinData(mesh);
	this->getVertexRemap(mesh);
	this->getMorphWeights(mesh);
	this->getMeshMapInfo(mesh);
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

	if (!mesh.skin.weights.empty())
		mesh.skin.numWeights = 4; // 2022 weights adhere to RGBA order or 4 indices per vtx

	/* Ignore data stream if using lightweight loader */
	if (m_parent->getLoadType() == enModelDefs::LoadLightWeight)
	{
		seekToEnd(m_data);
		return;
	}

	this->getVertexRemap(mesh);
	this->getMorphWeights(mesh);
	this->getMeshMapInfo(mesh);
}


void CSkinModel_1_1::buildMesh(Mesh& mesh)
{
	uint32_t index, numStacks;
	index = ReadUInt32(m_data);

	mesh.sceneFlag = ReadUInt32(m_data);
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

	if (!mesh.skin.weights.empty())
		mesh.skin.numWeights = 4; // 2022 weights adhere to RGBA order or 4 indices per vtx

	/* Ignore data stream if using lightweight loader */
	if (m_parent->getLoadType() == enModelDefs::LoadLightWeight)
	{
		seekToEnd(m_data);
		return;
	}

	this->getVertexRemap(mesh);
	seekToEnd(m_data);
	//this->getMorphWeights(mesh);
	//this->getMeshMapInfo(mesh);
}

void CSkinModel_1_1::loadModelBones(const uintptr_t& size)
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

void CSkinModel_1_1::readBone()
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

void CSkinModel_1_1::getMeshMapInfo(Mesh& mesh)
{
	int index = ReadUInt32(m_data);
	mesh.definition = m_stringTable.at(index);

	/* Load detail map info */
	loadColorMapInfo(mesh);
	seekToEnd(m_data);
}

void CSkinModel_1_1::getMorphWeights(Mesh& mesh)
{
	/* Load all vertex morphs */
	vCMeshShapes_2020 blendshapes(m_data, m_stringTable, &mesh);
	blendshapes.load();
}

void CSkinModel_2_15::loadMaterials()
{
	uint32_t numMats = ReadUInt32(m_data);

	for (int i = 0; i < numMats; i++) {
		Material mat;
		uint16_t index = ReadUInt16(m_data);
		uint32_t crc   = ReadUInt32(m_data);

		mat.name = m_stringTable.at(index);
		m_materials.push_back(mat);
	}
}

void CSkinModel_2_15::getTriangleBuffer(Mesh& mesh)
{
	std::vector<int> indices;
	uint16_t index = ReadUInt16(m_data);
	uint32_t numIndices = ReadUInt32(m_data);
	int numTriangles = numIndices / 3;

	/* Collect all face indices */
	for (int i = 0; i < numTriangles; i++) {
		Triangle face = (mesh.numVerts > 65535) ?
			Triangle{ ReadUInt32(m_data), ReadUInt32(m_data), ReadUInt32(m_data) } :
			Triangle{ ReadUInt16(m_data), ReadUInt16(m_data), ReadUInt16(m_data) };

		mesh.triangles.push_back(face);
	}

	/* Assign per face materials */
	m_data = MeshSerializer::Data::roundPointerToNearest4(m_data);
	uint32_t numMaterials = ReadUInt32(m_data);

	/* Collect all material face groups */
	for (int i = 0; i < numMaterials; i++)
	{
		FaceGroup mtlGroup;
		uint32_t mtlIndex     = ReadUInt32(m_data);
		mtlGroup.faceBegin    = ReadUInt32(m_data) / 3;
		mtlGroup.numTriangles = ReadUInt32(m_data) / 3;
		uint32_t unk          = ReadUInt32(m_data) / 3; // ???
		mtlGroup.material     = m_materials.at(mtlIndex);

		mesh.groups.push_back(mtlGroup);
	}

	m_data += 0xC;
}

