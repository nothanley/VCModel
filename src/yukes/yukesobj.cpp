#include <yukes/yukesobj.h>
#include "MemoryReader/memoryreader.h"
#include <glm/gtx/euler_angles.hpp>

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
	this->loadArmature(m_info.armature);

	for (int i = 0; i < m_info.numMeshes; i++)
	{
		this->readMesh();
	}
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
	uint32_t numGroups    = _u32;
	char*    weightTable  = m_info.ymxen + _u32;
	char*    faceTable    = m_info.ymxen + _u32;
	uint32_t index        = _u32;
	uint32_t unk2         = _u32;
	char*    vtxPosBf     = m_info.ymxen + _u32;
	char*    texCoordBf   = m_info.ymxen + _u32;
	uint32_t unk4         = _u32;
	uint32_t unk5         = _u32;
	uint32_t count        = _u32;
	uint32_t unk7         = _u32;
	
	// Load mesh streams
	char* meshDef = m_info.stringTbl + (index * 0x10);
	mesh->numVerts = count;
	mesh->name       = ReadString(meshDef, 0x10);
  	mesh->definition = mesh->name;

	::loadMeshAABBs(*mesh, m_data);
	  loadWeights(*mesh, weightTable, numWgtSegm);
	  loadTris(*mesh, numGroups, faceTable);
	  loadVerts(*mesh, vtxPosBf);
	  loadTexCoords(*mesh, nullptr);
	  //loadNorms(*mesh, nullptr);

	// push to scene
	mesh->skin.unpack(m_stringTable);
	m_meshes.push_back(mesh);
}

void CYukesSkinModel::loadArmature(char* stream)
{
	for (int i = 0; i < m_info.numBones; i++)
	{
		auto bone = this->loadBone(stream);
		bone->index = i;
		m_bones.push_back(bone);
		stream += 0x50;
	}
}

RigBone* CYukesSkinModel::loadBone(char* data)
{
	char* stream  = data;
	RigBone* bone = new RigBone;
	bone->name    = ReadString(stream, 0x10);
	m_stringTable.push_back(bone->name);

	stream = data + 0x10;
	glm::vec4 position ( ReadFloat(stream), ReadFloat(stream), ReadFloat(stream), ReadFloat(stream) );
	glm::vec4 rotation ( ReadFloat(stream), ReadFloat(stream), ReadFloat(stream), ReadFloat(stream) );
	int parent_index =   ReadInt32(stream);

	stream = data + 0x40;
	glm::vec4 scale( ReadFloat(stream), ReadFloat(stream), ReadFloat(stream), ReadFloat(stream) );

	position.x *=  scale.x;
	position.y *=  scale.y;
	position.z *=  scale.z;

	bone->matrix_local    = glm::eulerAngleXYZ(rotation.x, rotation.z, rotation.y);
	bone->matrix_local[3] = glm::vec4(position.x, position.z, -position.y, 1.0f);;
	bone->matrix_world    = bone->matrix_local; // Set transform basis

	if (parent_index >= 0)
	{
		bone->set_parent(m_bones.at(parent_index));
	}

	return bone;
}

void CYukesSkinModel::loadVerts(Mesh& mesh, char* stream)
{
	stream += 0x20; // skip unknown data section

	for (int i = 0; i < mesh.numVerts; i++)
	{
		mesh.vertices.push_back(ReadFloat(stream));
		mesh.vertices.push_back(ReadFloat(stream));
		mesh.vertices.push_back(ReadFloat(stream));
		stream += sizeof(float);
		//mesh.vertices.push_back(ReadFloat(stream));
	}
}

void CYukesSkinModel::loadNorms(Mesh& mesh, char* stream)
{
	for (int i = 0; i < mesh.numVerts; i++)
	{
		mesh.normals.push_back(1.0f);
		mesh.normals.push_back(1.0f);
		mesh.normals.push_back(1.0f);
		//stream += sizeof(float);
		//mesh.normals.push_back(1.0f);
	};
}

inline static void loadBlendWeights(Mesh& mesh, char* stream, const int size, const int numWeights)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			Vec4 data;

			if (j < numWeights) 
			{
				data = Vec4{ ReadFloat(stream), ReadFloat(stream), ReadFloat(stream), ReadFloat(stream) };
			}

			mesh.skin.weights.push_back(data.x);
		}
	}
};

inline static void loadBlendIndices(Mesh& mesh, char*& stream, const int size, const int numWeights)
{
	// Load weight indices
	std::vector<int> indices(4);

	for (int j = 0; j < 4; j++)
	{
		auto index = ReadUInt32(stream);
		indices[j] = index;
	}

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mesh.skin.indices.push_back(indices[j]);
		}
	};

};

void CYukesSkinModel::loadWeights(Mesh& mesh, char* table, const int segments)
{
	if (segments > 1) {
		mesh.skin.numWeights = 4;
	}

	for (int i = 0; i < segments; i++)
	{
		uint32_t numVerts   = ReadUInt32(table);
		uint32_t numWeights = ReadUInt32(table);
		uint32_t unk		= ReadUInt32(table);
		char*    weightBf   = m_info.ymxen + ReadUInt32(table);

		if (numWeights > 4)
			throw std::runtime_error("Too many weights");

		// Load weight values
		::loadBlendIndices(mesh, table, numVerts, numWeights);
		::loadBlendWeights(mesh, weightBf, numVerts, numWeights);
	}
}

void CYukesSkinModel::loadTexCoords(Mesh& mesh, char* stream)
{
}

inline static void
stripsToTriangleList(std::vector<Triangle>& tris, const std::vector<int>& stripIndices)
{
	// A triangle strip needs at least 3 indices to form a valid triangle
	if (stripIndices.size() < 3)
		return;

	for (size_t i = 2; i < stripIndices.size(); ++i)
	{
		Triangle tri;

		if (i % 2 == 0) {
			// Even index: maintain winding order
			tri = Triangle{ 
				   static_cast<uint32_t>(stripIndices[i - 2] ),
				   static_cast<uint32_t>(stripIndices[i - 1] ),
				   static_cast<uint32_t>(stripIndices[i]     ) };
		}
		else {
			// Odd index: swap winding order
			tri = Triangle{ 
				   static_cast<uint32_t>(stripIndices[i - 2] ),
				   static_cast<uint32_t>(stripIndices[i]     ),
				   static_cast<uint32_t>(stripIndices[i - 1] ) };
		}

		tris.push_back(tri);
	}
}


inline static void loadMeshTriBf(Mesh& mesh, char* stream, std::vector<int>& indices, const int numA, const int numB, const int size)
{
	indices.reserve(size * 3);

	for (int i = 0; i < size; i++)
	{
		float unk0  = ReadFloat  (stream);
		float unk1  = ReadFloat  (stream);
		float unk2  = ReadFloat  (stream);
		int   index = ReadUInt32 (stream);
		float unk3  = ReadFloat  (stream);
		float unk4  = ReadFloat  (stream);
		float unk5  = ReadFloat  (stream);
		float unk6  = ReadFloat  (stream);

		indices.push_back(index);
	}
}

void CYukesSkinModel::loadTris(Mesh& mesh, const int numGroups, char* stream)
{
	mesh.groups.resize(numGroups);

	for (int k = 0; k < numGroups; k++)
	{
		auto& group = mesh.groups[k];
		stream += 0xC0;

		group.material.name = mesh.name;
		group.faceBegin     = mesh.triangles.size();

		int16_t  faceType    = ReadInt16(stream);
		int16_t  numIndices  = ReadInt16(stream);
		uint32_t numSegments = ReadUInt32(stream);
		char* faceTable      = m_info.ymxen + ReadUInt32(stream);
		uint32_t endOffset   = ReadUInt32(stream);

		for (int i = 0; i < numSegments; i++)
		{
			uint32_t unk0 = ReadUInt32(faceTable);
			uint32_t unk1 = ReadUInt32(faceTable);
			uint32_t numTris = ReadUInt32(faceTable);
			char* data = m_info.ymxen + ReadUInt32(faceTable);

			std::vector<int> indices;
			::loadMeshTriBf(mesh, data, indices, unk0, unk1, numTris);
			::stripsToTriangleList(mesh.triangles, indices);
		}

		// Calculate group size
		group.numTriangles = mesh.triangles.size() - group.faceBegin;
	}
}



