#include "blendshapes_legacy.h"
#include <BinaryIO.h>
#include "meshbuffers.h"

using namespace BinaryIO;
using namespace MeshSerializer;

vCMeshShapes_2020::vCMeshShapes_2020(
	char*& data, 
	const std::vector<std::string>& table, 
	Mesh* mesh) 
	: vCMeshShapes(data, table, mesh)
{
}

void
vCMeshShapes_2020::load()
{
	if (!m_data)
		return;

	/* Collect all blendshape names */
	this->getMorphs();
}

void
vCMeshShapes_2020::getMorphs()
{
	this->m_numMorphs = ReadUInt32(m_data);

	/* Aquire all blend shape id's */
	for (int i = 0; i < m_numMorphs; i++) 
	{
		StBlendShape sBlendShape;
		int index = ReadUInt32(m_data);
		sBlendShape.name = m_stringTable.at(index);
		sBlendShape.vertices = m_mesh->vertices;

		this->loadBlendshapeData(sBlendShape);
		m_shapes->push_back(sBlendShape);
	}

	/* Align stream */
	m_data = Data::roundPointerToNearest4(m_data);
}

void
vCMeshShapes_2020::loadBlendshapeData(StBlendShape& shape)
{
	uint32_t numDeltas = ReadUInt32(m_data);

	for (int i = 0; i < numDeltas; i++)
	{
		Vec2 delta;

		delta.x = ReadUInt16(m_data);
		delta.y = ReadUInt32(m_data);
	}
}


