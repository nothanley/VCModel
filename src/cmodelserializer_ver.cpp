#include <cmodelserializer_ver.h>
#include <cmodelserializer.h>
#include <skinmodel.h>
#include <meshencoder.h>
#include <meshtags.h>
#include "MemoryReader/memoryreader.h"
#include "glm/gtx/euler_angles.hpp"
#include "winsock.h"
#include <algorithm>
#include <unordered_set>
#include <cmath>

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

struct EdgeLodStat
{
	uint32_t edgeCount = 0;
	float maxEdge = 0.0f;
	float avgEdge = 0.0f;
	float minEdge = 0.0f;
	float medianEdge = 0.0f;
};

static uint64_t make_edge_key(uint32_t a, uint32_t b)
{
	uint32_t lo = (a < b) ? a : b;
	uint32_t hi = (a < b) ? b : a;
	return (uint64_t(hi) << 32) | uint64_t(lo);
}

static EdgeLodStat compute_edge_lod_stat(const Mesh* mesh)
{
	EdgeLodStat out{};
	if (!mesh || mesh->triangles.empty()) return out;

	std::unordered_set<uint64_t> edges;
	edges.reserve(mesh->triangles.size() * 3);
	std::vector<float> lengths;
	lengths.reserve(mesh->triangles.size() * 3);

	auto push_edge = [&](uint32_t a, uint32_t b)
	{
		uint64_t key = make_edge_key(a, b);
		if (!edges.insert(key).second) return;

		Vec3 va = mesh->vertex(static_cast<int>(a));
		Vec3 vb = mesh->vertex(static_cast<int>(b));
		Vec3 d  = va - vb;
		float len = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
		lengths.push_back(len);
	};

	for (const auto& tri : mesh->triangles)
	{
		push_edge(tri[0], tri[1]);
		push_edge(tri[1], tri[2]);
		push_edge(tri[2], tri[0]);
	}

	if (lengths.empty()) return out;

	out.edgeCount = static_cast<uint32_t>(lengths.size());
	out.maxEdge = *std::max_element(lengths.begin(), lengths.end());
	out.minEdge = *std::min_element(lengths.begin(), lengths.end());

	double sum = 0.0;
	for (float v : lengths) sum += v;
	out.avgEdge = static_cast<float>(sum / lengths.size());

	std::sort(lengths.begin(), lengths.end());
	size_t mid = lengths.size() / 2;
	out.medianEdge = (lengths.size() % 2 == 1)
		? lengths[mid]
		: (lengths[mid - 1] + lengths[mid]) * 0.5f;

	return out;
}

static float compute_edge_lod_bias(const Mesh* mesh, float k)
{
	if (!mesh || mesh->triangles.empty()) return 0.0f;
	return k / static_cast<float>(mesh->triangles.size());
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

void CModelSerializer_2_15::serialize()
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

void CModelSerializer_2_15::createModelBuffer()
{
	StModelBf stream;

	stream.type = "MDL!";
	stream.size = getMDLBufferSize();
	stream.data = new char[stream.size];
	char* buffer = stream.data;
	bool use_rig = m_model->getNumBones();

	WriteUInt32(buffer, 0x2F); // File format version
	WriteUInt32(buffer, use_rig); // Unknown value
	writeBoundingBox(buffer, m_model->getAABBs()); // Model bounds

	m_dataBuffers.push_back(stream);
}

void CModelSerializer_2_15::createMaterialBuffer()
{
	/* Initialize model buffer stream */
	const auto& meshes = m_model->getMeshes();
	uint32_t numMeshes = meshes.size();

	StModelBf stream;
	stream.type = "MTL!";
	stream.size = getMtlBufferSize(meshes);
	stream.data = new char[stream.size];
	char* buffer = stream.data;

	WriteUInt32(buffer, numMeshes);
	for (auto& mesh : meshes) 
	{
		int16_t index = -1;
		uint32_t crc  = 0;

		if (mesh->groups.size() > 0)
		{
			FaceGroup& group = mesh->groups.front();
			index = indexOf(group.material.name);

			crc = CSerializedModel::getStringCrc(group.material.name, true); // unchecked if str should be lower - verify this
		}

		WriteUInt16(buffer, index);
		WriteUInt32(buffer, crc);
	}

	::align_binary_stream(buffer);
	m_dataBuffers.push_back(stream);
}


void CModelSerializer_2_15::writeMaterialGroupBuffer(char*& buffer, int meshIndex)
{
	/* Write material groups */
	auto mesh = m_model->getMeshes().at(meshIndex);
	int numGroups = mesh->groups.size();
	WriteUInt32(buffer, numGroups);

	for (int i = 0; i < numGroups; i++)
	{
		auto& group = mesh->groups.at(i);
		uint32_t maxVertexIndex = 0;
		if (group.numTriangles > 0 &&
			static_cast<size_t>(group.faceBegin + group.numTriangles) <= mesh->triangles.size())
		{
			for (int t = 0; t < group.numTriangles; t++)
			{
				const auto& tri = mesh->triangles.at(group.faceBegin + t);
				if (tri[0] > maxVertexIndex) maxVertexIndex = tri[0];
				if (tri[1] > maxVertexIndex) maxVertexIndex = tri[1];
				if (tri[2] > maxVertexIndex) maxVertexIndex = tri[2];
			}
		}
		WriteUInt32(buffer, meshIndex);		// material index
		WriteUInt32(buffer, group.faceBegin * 3);
		WriteUInt32(buffer, group.numTriangles * 3);
		WriteUInt32(buffer, maxVertexIndex);
	}

	/* Write ENDM tag */
	WriteUInt32(buffer, 0); // pad
	WriteUInt64(buffer, ntohl(ENDM));
}


uint32_t 
CModelSerializer_2_15::getMtlBufferSize(const std::vector<Mesh*>& meshes)
{
	uint32_t size = sizeof(uint32_t);
	size += ((sizeof(uint16_t) + sizeof(uint32_t)) * meshes.size());
	::round_size(size, 4); // align

	return size;
}

void 
CModelSerializer_2_15::updateIndexBufferSize(uint32_t& size, const Mesh* mesh)
{
	int encodeWidth = (mesh->numVerts > UINT16_MAX) ? sizeof(uint32_t) : sizeof(uint16_t);
	size += sizeof(uint16_t); // Mesh Index
	size += sizeof(uint32_t); // Num Faces
	size += encodeWidth * (mesh->triangles.size() * 3); // Index buffer
	::round_size(size, 4); // align

	size += sizeof(uint32_t); // Num Material Groups
	size += mesh->groups.size() * (sizeof(uint32_t) * 4); // mtl index, faceBegin, faceEnd
	size += sizeof(uint32_t); // pad
	size += sizeof(uint64_t); // ENDM Tag
}


void 
CModelSerializer_2_15::serializeVertices(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "POSITION", "R32_G32_B32", "float");

	/* Write vertex buffer */
	std::vector<float>& vertices = target.mesh->vertices;
	dataBf->stream.write((char*)vertices.data(), sizeof(float) * vertices.size());

	target.data.push_back(dataBf);
}

void 
CModelSerializer_2_15::serializeVertexNormals(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "NORMAL", "R32_G32_B32", "float");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& normals = target.mesh->normals;
	int array_size = (normals.size() == target.mesh->vertices.size()) ? 3 : 4;

	for (int i = 0; i < normals.size(); i += array_size) {
		Vec3 normal{ normals[i], normals[i + 1], normals[i + 2] };

		WriteFloat(stream, normal.x);
		WriteFloat(stream, normal.y);
		WriteFloat(stream, normal.z);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void 
CModelSerializer_2_15::serializeTangents(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "TANGENT", "R32_G32_B32", "float");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& tangents = target.mesh->tangents;
	uint8_t array_size = (tangents.size() == target.mesh->vertices.size()) ? 3 : 4;

	for (int i = 0; i < tangents.size(); i += array_size)
	{
		Vec3 tangent{ tangents[i], tangents[i + 1], tangents[i + 2] };

		WriteFloat(stream, tangent.x);
		WriteFloat(stream, tangent.y);
		WriteFloat(stream, tangent.z);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void CModelSerializer_2_15::writeUvDictTail(std::stringstream& stream, Mesh* mesh)
{
	EdgeLodStat lod0  = compute_edge_lod_stat(mesh);
	uint32_t lodCount = (m_numLods > 0) ? m_numLods : 1;
	const float kBiasA = 0.00569339f;
	const float kBiasB = 0.00569648f;
	const float biasA = compute_edge_lod_bias(mesh, kBiasA);
	const float biasB = compute_edge_lod_bias(mesh, kBiasB);

	WriteFloat  ( stream, biasA    );
	WriteFloat  ( stream, biasB    );
	WriteUInt32 ( stream, lodCount );

	for (uint32_t i = 0; i < lodCount; ++i)
	{
		WriteUInt32 ( stream, lod0.edgeCount  );
		WriteFloat  ( stream, lod0.maxEdge    );
		WriteFloat  ( stream, lod0.avgEdge    );
		WriteFloat  ( stream, lod0.minEdge    );
		WriteFloat  ( stream, lod0.medianEdge );
	}
}

