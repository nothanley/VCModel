#include "meshencoder.h"
#include <skinmodel.h>
#include "cmeshserializer.h"
#include "meshbuffers.h"

uint32_t MeshEncoder::getMeshBufferDefSize(std::vector<StMeshBf>& meshbuffers)
{
	uint32_t size = sizeof(uint32_t); // Num Meshes
	for (auto& mesh : meshbuffers)
	{
		size += sizeof(uint32_t) * 3; // index + flags
		size += sizeof(uint16_t);	  // Null
		size += sizeof(uint32_t) * 6; // mesh AABBs
		size += sizeof(uint32_t) * 2; // numverts + streams

		for (auto& child : mesh.data)
			size += child->size();

		size += sizeof(uint32_t) * 2; // ENDM magic
	}

	return size;
}


uint32_t MeshEncoder::getMtlBufferSize(const std::vector<Mesh*>& meshes)
{
	uint32_t size = sizeof(uint32_t);
	size += (sizeof(uint32_t) * meshes.size());
	return size;
}

uint32_t MeshEncoder::getBoneBufferSize(const std::vector<RigBone*>& bones)
{
	uint32_t tableLength = sizeof(uint32_t) * 4;     // Varies with revision type
	uint32_t entrySize = sizeof(uint16_t) * 2;		 // index + parent
	entrySize += sizeof(uint32_t) * 24;				 // translate + rotation vectors
	entrySize += sizeof(uint8_t) + sizeof(uint32_t); // Unknown values - new
	entrySize *= bones.size();

	return tableLength + entrySize;
}

uint32_t MeshEncoder::getStringBufferSize(const std::vector<std::string>& strings) {
	uint32_t size = sizeof(uint32_t);
	size += (strings.size() * sizeof(uint32_t));

	for (auto& string : strings) {
		size += (string.size() + 1);
	}
	return size;
}

uint32_t MeshEncoder::getLodsBufferSize(const std::vector<Mesh*>& meshes, int numLevels) 
{
	int numMeshes = meshes.size();
	uint32_t size = sizeof(uint32_t); // Num LOD Levels

	// Only encodes high+low LODs. 
	for (int i = 0; i < numLevels; i++) {
		size += sizeof(uint32_t); // Num meshes;
		for (auto& mesh : meshes){
			int encodeWidth = (mesh->numVerts > UINT16_MAX) ? sizeof(uint32_t) : sizeof(uint16_t);
			size += sizeof(uint16_t); // Mesh Index
			size += sizeof(uint32_t); // Num Faces
			size += encodeWidth * (mesh->triangles.size() * 3); // Index buffer
			while (size % 4 != 0) size++; // align

			size += sizeof(uint32_t); // Num Material Groups
			for (auto& group : mesh->groups) {
				size += sizeof(uint32_t) * 3; // mtl index, faceBegin, faceEnd
			}

			size += sizeof(uint64_t); // ENDM Tag
		}
	}

	return size;
}

uint32_t MeshEncoder::getMDLBufferSize()
{
	uint32_t size = 0;

	size += sizeof(uint32_t); // MDL format version
	size += sizeof(uint32_t); // unknown flag
	size += sizeof(uint32_t) * 6; // Overall AABBs
	return size;
}
