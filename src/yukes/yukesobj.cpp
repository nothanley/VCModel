#include <yukes/yukesobj.h>
#include "MemoryReader/memoryreader.h"

using namespace memreader;

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

inline static void loadMeshAABBs(Mesh& mesh, char* m_data)
{
	mesh.bounds.minX   = _float;
	mesh.bounds.minY   = _float;
	mesh.bounds.minZ   = _float;
	mesh.bounds.radius = _float;
};

void CYukesSkinModel::readMesh()
{
	Mesh mesh;

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
	uint32_t unk6         = _u32;
	uint32_t unk7         = _u32;
	
	// Load mesh streams
	::loadMeshAABBs(mesh, m_data);
	  loadWeights(mesh, weightTable, numWgtSegm);
}

void CYukesSkinModel::readArmature()
{

}

void CYukesSkinModel::readBone()
{

}

void CYukesSkinModel::loadVerts()
{

}

void CYukesSkinModel::loadNorms()
{

}

void CYukesSkinModel::loadTris()
{

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

inline static void loadBlendIndices(Mesh& mesh, char* stream, const int numWeights)
{
	// Load weight indices
	for (int j = 0; j < numWeights; j++)
	{
		auto index = ReadUInt32(stream);
		mesh.skin.indices.push_back(index);
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
		::loadBlendIndices(mesh, m_data, numWeights);
		::loadBlendWeights(mesh, weightBf);
	}
}

void CYukesSkinModel::loadTexCoords()
{

}