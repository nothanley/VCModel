#include <yukes/yukesobj.h>
#include "MemoryReader/memoryreader.h"

using namespace BinaryIO;

#define _u64   ReadUInt64(m_data)
#define _u32   ReadUInt32(m_data)
#define _u16   ReadUShort(m_data)
#define _u8    ReadByte(m_data)
#define _s32   ReadSInt32(m_data)
#define _bool  ReadBool(m_data)
#define _float ReadFloat(m_data)


CYukesSkinModel::CYukesSkinModel(char* data, CModelContainer* pParent) 
	: CSkinModel(data, pParent)
{
	this->loadData();
}

void CYukesSkinModel::loadData()
{
	m_info.ymxen = m_data;
	this->readHeader();

	for (int i = 0; i < m_info.numMeshes; i++)
	{
		this->readMesh();
	}

	printf("");
}

void CYukesSkinModel::readHeader()
{
	auto& m = m_info;

	m_data      += 0x4;
	m.pof0       = m.ymxen + _u32;
	m_data      += 0x8;
	m.numMeshes  = _u32;
	m.numBones   = _u32;
	m.numTexs    = _u32;
	m.meshTable  = m.ymxen + _u32;
	m.armature   = m.ymxen + _u32;
	m.texTable   = m.ymxen + _u32;
	m.stringTbl  = m.ymxen + _u32;
	m.numStrings = _u32;
	m_data      += 0x10;
}

inline static void loadMeshAABBs(Mesh& mesh, char*& m_data)
{
	mesh.bounds.minX   = _float;
	mesh.bounds.minY   = _float;
	mesh.bounds.minZ   = _float;
	mesh.bounds.radius = _float;
};

void CYukesSkinModel::readMesh()
{
	Mesh* mesh = new Mesh; // todo: change this from a raw pointer ...

	// Load mesh table data
	uint32_t numWgtSegm   = _u32;
	uint32_t unk          = _u32;
	char*    weightTable  = m_info.ymxen + _u32;
	char*    faceTable    = m_info.ymxen + _u32;
	uint32_t unk2         = _u32;
	uint32_t unk3         = _u32;
	char*    vtxPosBf     = m_info.ymxen + _u32;
	char*    texCoordBf   = m_info.ymxen + _u32;
	uint32_t unk4         = _u32;
	uint32_t unk5         = _u32;
	uint32_t count        = _u32;
	uint32_t unk7         = _u32;
	
	// Load mesh streams
	mesh->numVerts = count;

	::loadMeshAABBs(*mesh, m_data);
	  loadWeights(*mesh, weightTable, numWgtSegm);
	  loadTris(*mesh, faceTable);
	  loadVerts(*mesh, vtxPosBf);
	  loadTexCoords(*mesh, nullptr);
	  loadNorms(*mesh, nullptr);

	// push to scene
	m_meshes.push_back(mesh);
}

void CYukesSkinModel::readArmature()
{
}

void CYukesSkinModel::readBone()
{
}

void CYukesSkinModel::loadVerts(Mesh& mesh, char* stream)
{
	stream += 0x20; // skip unknown data section

	for (int i = 0; i < mesh.numVerts; i++)
	{
		mesh.vertices.push_back(ReadFloat(stream));
		mesh.vertices.push_back(ReadFloat(stream));
		mesh.vertices.push_back(ReadFloat(stream));
		mesh.vertices.push_back(ReadFloat(stream));
	}
}

void CYukesSkinModel::loadNorms(Mesh& mesh, char* stream)
{
	for (int i = 0; i < mesh.numVerts; i++)
	{
		mesh.normals.push_back(1.0f);
	};
}

inline static void loadBlendWeights(Mesh& mesh, char* stream, const int numWeights, const int size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < numWeights; j++)
		{
			Vec4 data{ ReadFloat(stream), ReadFloat(stream), ReadFloat(stream), ReadFloat(stream) };
			mesh.skin.weights.push_back(data.x);
		}
	}
};

inline static void loadBlendIndices(Mesh& mesh, char*& stream, const int numWeights)
{
	// Load weight indices
	for (int j = 0; j < 4; j++)
	{
		auto index = ReadUInt32(stream);

		if (j < numWeights)
		{
			mesh.skin.indices.push_back(index);
		}
	}
};

void CYukesSkinModel::loadWeights(Mesh& mesh, char* table, const int segments)
{
	for (int i = 0; i < segments; i++)
	{
		uint32_t numVerts   = ReadUInt32(table);
		uint32_t numWeights = ReadUInt32(table);
		uint32_t unk		= ReadUInt32(table);
		char*    weightBf   = m_info.ymxen + ReadUInt32(table);

		if (numWeights > 4)
			throw std::runtime_error("Too many weights");

		// Load weight values
		::loadBlendIndices(mesh, table, numWeights);
		::loadBlendWeights(mesh, weightBf, numVerts, numWeights);
	}
}

void CYukesSkinModel::loadTexCoords(Mesh& mesh, char* stream)
{
}

inline static
std::vector<Triangle> stripsToIndices(const std::vector<int>& stripIndices) 
{
	std::vector<Triangle> tris;

	// Ensure there are enough indices to form triangles
	if (stripIndices.size() < 3) {
		return tris; // Not enough indices to form any triangles
	}

	// Iterate through the strip and create triangles
	for (size_t i = 2; i < stripIndices.size(); ++i) 
	{
		Triangle tri{ (stripIndices[i - 2]),
			          (stripIndices[i - 1]), 
			          (stripIndices[i])  };
		tris.push_back(tri);

		// For the next triangle, we need to alternate the vertex order
		if (i % 2 == 0)
		{
			Triangle tri{ (stripIndices[i - 2]),
			              (stripIndices[i - 1]), 
			              (stripIndices[i]) };

			tris.push_back(tri);
		}
		else
		{
			Triangle tri{ (stripIndices[i - 1]),
			              (stripIndices[i - 2]), 
			              (stripIndices[i]) };
			tris.push_back(tri);
		}
	}

	return tris;
}



inline static void loadMeshTriBf(Mesh& mesh, char* stream, std::vector<int>& indices, const int numA, const int numB, const int size)
{
	indices.reserve(size * 3);

	for (int i = 0; i < size; i++)
	{
		float unk0  = ReadFloat(stream);
		float unk1  = ReadFloat(stream);
		float unk2  = ReadFloat(stream);
		int   index = ReadUInt32(stream);
		float unk3  = ReadFloat(stream);
		float unk4  = ReadFloat(stream);
		float unk5  = ReadFloat(stream);
		float unk6  = ReadFloat(stream);
		indices.push_back(index);
	}
}

void CYukesSkinModel::loadTris(Mesh& mesh, char* stream)
{
	stream += 0xC0; // Skip unknown data section

	// load lod table
	int16_t  faceType    = ReadInt16(stream);
	int16_t  numIndices  = ReadInt16(stream);
	uint32_t numSegments = ReadUInt32(stream);
	char* faceTable      = m_info.ymxen + ReadUInt32(stream);
	uint32_t unkOff      = ReadUInt32(stream);

	std::vector<int> indices;
	for (int i = 0; i < numSegments; i++)
	{
		uint32_t unk0 = ReadUInt32(faceTable);
		uint32_t unk1 = ReadUInt32(faceTable);
		uint32_t numTris = ReadUInt32(faceTable);
		char* data = m_info.ymxen + ReadUInt32(faceTable);
		::loadMeshTriBf(mesh, data, indices, unk0, unk1, numTris);
	}

	mesh.triangles = ::stripsToIndices(indices);
}



