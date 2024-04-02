#include "BinaryIO.h"
#include "meshtags.h"
#include "databuffers.h"
#include "skinmodelpoly.h"
using namespace BinaryIO;

void 
CSkinModel_2_8::loadBuffer()
{
	uint32_t type = ReadUInt32(m_data);
	uint32_t size = ReadUInt32(m_data);
	char* nextBfPtr = m_data + size;

	// Handle stream
	switch (type) 
	{
		case TEXT:
			CDataBuffer::getStringTable(m_data, m_stringTable);
			break;
		case BONE:
			CDataBuffer::getModelBones_2_8(m_data, size, m_bones, m_stringTable);
			break;
		case MTL:
			CDataBuffer::getMaterials(m_data, size, m_materials, m_stringTable);
			break;
		case MBfD:
			CDataBuffer::getMeshes(m_data, size, m_meshes, m_stringTable);
			break;
		case LODs:
			CDataBuffer::getLods(m_data, size, m_meshes, m_materials, m_stringTable);
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
			CDataBuffer::getStringTable(m_data, m_stringTable);
			break;
		case BONE:
			CDataBuffer::getModelBones_2_5(m_data, size, m_bones, m_stringTable);
			break;
		case MTL:
			CDataBuffer::getMaterials(m_data, size, m_materials, m_stringTable);
			break;
		case MBfD:
			CDataBuffer::getMeshes(m_data, size, m_meshes, m_stringTable);
			break;
		case LODs:
			CDataBuffer::getLods(m_data, size, m_meshes, m_materials, m_stringTable);
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
