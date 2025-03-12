#include <cmodelserializer_ver.h>
#include <cmodelserializer.h>
#include <skinmodel.h>
#include <meshencoder.h>
#include <meshtags.h>
#include "MemoryReader/memoryreader.h"
#include "glm/gtx/euler_angles.hpp"
#include "winsock.h"
#include <algorithm>

using namespace memreader;

inline static int getNumJigBones(const std::vector<RigBone*>& bones)
{
	int jigCount = 0;
	for (auto& bone : bones)
		if (bone->jig && (bone->jig->index >= 0))
			jigCount++;

	return jigCount;
}

inline static bool contains(std::string str, std::string substr) 
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::transform(substr.begin(), substr.end(), substr.begin(), ::tolower);
	return str.find(substr) != std::string::npos;
}

inline static bool startsWith(std::string str, std::string prefix)
{
	if (str.length() < prefix.length()) return false;
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
	return str.substr(0, prefix.length()) == prefix;
}

void CModelSerializer_2_9::createModelBuffer()
{
	StModelBf stream;

	stream.type  = "MDL!";
	stream.size  = getMDLBufferSize();
	stream.data  = new char[stream.size];
	char* buffer = stream.data;
	bool use_rig = m_model->getNumBones();

	WriteUInt32(buffer, 0x29); // File format version
	WriteUInt32(buffer, use_rig); // Unknown value
	writeBoundingBox(buffer, m_model->getAABBs()); // Model bounds

	m_dataBuffers.push_back(stream);
}

void CModelSerializer_2_9::writeDataBuffer(std::ofstream& fs, const StModelBf& modelBf)
{
	fs.write(modelBf.type.c_str(), 0x4); // Stream Magic

	/* Write stream size info */
	if (modelBf.type != "MDL!" && modelBf.type != "MCD!")
		WriteUInt32(&fs, modelBf.size);

	/* Push stream binary to file stream */
	fs.write(modelBf.data, modelBf.size);
}

inline void writeGenericHpl(std::ofstream* file)
{
	WriteUInt32(file, HPL);
	WriteUInt32(file, 0);
	WriteUInt64(file, END);
}

void CModelSerializer_2_9::formatFile()
{
	std::ofstream file(m_savePath, std::ios::binary);
	if (!file.is_open())
		return;

	/* Push all data streams to disk file */
	for (auto& dataBf : m_dataBuffers) {
		writeDataBuffer(file, dataBf);
		dataBf.free(); // Clean MDL data struct
	}

	// Update MDL! size info
	size_t size = file.tellp();
	file.seekp(0x8);
	WriteUInt32(&file, size);

	// Add footer tag to model buffer
	file.seekp(size);
	::writeGenericHpl(&file);
	WriteUInt64(&file, 0x4); // Write tag
	WriteUInt32(&file, END); // Write END! tag
	file.close();
}

void CModelSerializer_2_9::createMCDBuffer()
{
	StModelBf stream;

	stream.type = "MCD!";
	stream.size = 12;
	stream.data = new char[stream.size];
	char* buffer = stream.data;

	WriteUInt32(buffer, 0x4);         // File format version
	WriteUInt32(buffer, stream.size); // MDL Size
	WriteUInt32(buffer, MDL_MAGIC);   // MDL Tag

	m_dataBuffers.push_back(stream);
}

void CModelSerializer_2_9::serialize()
{
	this->generateStringTable();

	this->createMCDBuffer();
	this->createModelBuffer();
	this->createTextBuffer();
	this->createBoneBuffer();
	this->createAtPtBuffer();
	this->createMaterialBuffer();
	this->createMeshBufferDefs();
	this->createLODsBuffer();

	this->formatFile();
}

void CModelSerializer_2_9::writeMeshBuffer(char*& buffer, const StMeshBf& meshBuffer)
{
	auto& mesh = meshBuffer.mesh;
	WriteUInt32(buffer, indexOf(mesh->name));
	WriteUInt32(buffer, mesh->sceneFlag);
	WriteUInt32(buffer, 0); // alignment
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

void CModelSerializer_2_9::serializeVertices(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "POSITION", "R32_G32_B32", "float");

	/* Write vertex buffer */
	std::vector<float>& vertices = target.mesh->vertices;
	dataBf->stream.write((char*)vertices.data(), sizeof(float) * vertices.size());

	target.data.push_back(dataBf);
}

uint32_t CModelSerializer_2_9::getMeshBufferDefSize(std::vector<StMeshBf>& meshbuffers)
{
	uint32_t size = sizeof(uint32_t); // Num Meshes
	for (auto& mesh : meshbuffers)
	{
		size += sizeof(uint32_t) * 3; // index + flags
		size += sizeof(uint32_t);	  // Null
		size += sizeof(uint32_t) * 6; // mesh AABBs
		size += sizeof(uint32_t) * 2; // numverts + streams

		for (auto& child : mesh.data)
			size += child->size();

		size += sizeof(uint32_t) * 2; // ENDM magic
	}

	return size;
}

void CModelSerializer_2_9::generateMeshBuffers(std::vector<StMeshBf>& buffers)
{
	const auto& meshes = m_model->getMeshes();

	for (auto& targetMesh : meshes) {
		StMeshBf meshbuffer;
		meshbuffer.mesh = targetMesh;

		serializeVertices(meshbuffer);
		serializeVertexNormals(meshbuffer);
		serializeTangents(meshbuffer);
		serializeBinormals(meshbuffer);
		serializePackedTbn(meshbuffer); // 2k24+
		serializeVertexColors(meshbuffer);
		serializeTexCoords(meshbuffer);
		serializeSkin(meshbuffer);
		serializeVertexRemap(meshbuffer);
		serializeBlendShapes(meshbuffer);
		serializeColorDict(meshbuffer);
		serializeUVDict(meshbuffer); // 2K24+

		buffers.push_back(meshbuffer);
	}
}

uint32_t CModelSerializer_2_9::getBoneBufferSize(const std::vector<RigBone*>& bones)
{
	uint32_t dataLen = sizeof(uint32_t) * 4; // Table header ; Varies with revision type

	for (int i = 0; i < bones.size(); ++i)
	{
		dataLen += sizeof(uint16_t) * 2; // index + parent
		dataLen += sizeof(uint32_t) * 6; // translate + rotation vectors
		dataLen += sizeof(uint8_t);      // use jiggle bone
		dataLen += sizeof(uint32_t);     // jiggle index
	}
	
	int jigCount = ::getNumJigBones(bones);
	for (int j = 0; j < jigCount; ++j)
	{
		dataLen += (sizeof(uint32_t) * 2); // jiggle infos
		dataLen += (sizeof(float) * 6);    // jiggle weights
	}

	::round_size(dataLen, 4); // align binary stream
	return dataLen;
}

inline static void writeJigBoneData(char*& buffer, std::shared_ptr<JigParam>& jig)
{
	/* Write bone jig info */
	if (!jig) return;

	WriteUInt32(buffer, jig->unk1);
	WriteUInt32(buffer, jig->unk2);
	WriteFloat(buffer, jig->weightsA.x);
	WriteFloat(buffer, jig->weightsA.y);
	WriteFloat(buffer, jig->weightsA.z);
	WriteFloat(buffer, jig->weightsB.x);
	WriteFloat(buffer, jig->weightsB.y);
	WriteFloat(buffer, jig->weightsB.z);
}

void CModelSerializer_2_9::createBoneBuffer()
{
	/* Initialize model buffer stream */
	std::vector<RigBone*> bones  = m_model->getBones();
	doJigBoneCheck(bones); // Verify all jig bones are set

	uint32_t numBones    = bones.size();
	uint32_t numJigBones = ::getNumJigBones(bones);
	if (numBones == 0) return;

	StModelBf stream;
	stream.type = "BONE";
	stream.size = getBoneBufferSize(bones);
	stream.data = new char[stream.size];
	
	/* Write buffer table */
	char* buffer = stream.data;
	WriteUInt32(buffer, 0);           // Unknown Data Float
	WriteUInt32(buffer, 0);           // Unknown Data Float
	WriteUInt32(buffer, numBones);    // Number of bones
	WriteUInt32(buffer, numJigBones); // Number of jig bones

	/* Write all bone data */
	for (auto& bone : bones) 
	{
		auto& jig           = bone->jig;
		int16_t boneIndex   = indexOf(bone->name);
		int16_t parentIndex = (bone->parent) ? indexOf(bone->parent->name) : -1;

		/* Write bone index values*/
		WriteUInt16(buffer, boneIndex);
		WriteUInt16(buffer, parentIndex);
		writeMatrixToBuffer(buffer, bone->matrix_local);
		int jigIndex = (jig) ? jig->index : -1;

		/* push new bone flags */
		WriteUInt8  (buffer, 0);
		WriteUInt32 (buffer, jigIndex);

		/* Write bone jig info */
		::writeJigBoneData(buffer, jig);
	}

	::align_binary_stream(buffer);
	m_dataBuffers.push_back(stream);
}

void CModelSerializer_2_9::doJigBoneCheck(std::vector<RigBone*>& bones)
{
	int glob_jindex(NULL);

	for (auto& bone : bones)
	{
		if ( !bone->jig && 
			::contains(bone->name, "_jig_") &&
			::startsWith(bone->name, "h_"))
		{
			bone->jig        = std::make_shared<JigParam>();
			bone->jig->index = glob_jindex;
			glob_jindex++;
		}
	}
}

