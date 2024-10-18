#include "blendshapes.h"
#include "meshbuffers.h"
#include "MemoryReader/memoryreader.h"

using namespace BinaryIO;
using namespace MeshSerializer;

vCMeshShapes::vCMeshShapes(char*& data, const std::vector<std::string>& table, Mesh* mesh) :
	m_data(data),
	m_stringTable(table),
	m_mesh(mesh),
	m_shapes(&mesh->blendshapes)
{}

inline
void vCMeshShapes::getTransformDeltas()
{
	this->m_numMorphVerts = ReadUInt32(m_data);

	this->m_tfmDelta  = Vec3{ ReadFloat(m_data),
						ReadFloat(m_data), 
						ReadFloat(m_data) };

	this->m_tfmLowest = Vec3{ ReadFloat(m_data),
						ReadFloat(m_data),
						ReadFloat(m_data) };
}

void 
vCMeshShapes::divmod(int a, int b, int* quotient, int* remainder)
{
	*quotient = a / b;
	*remainder = a % b;
}


uint8_t 
vCMeshShapes::reverseBits(uint8_t n) {
	// Reverse the top and bottom nibble then swap them.
	return (lookup[n & 0b1111] << 4) | lookup[n >> 4];
}

uint32_t 
vCMeshShapes::flip32bits(uint32_t value)
{
	uint32_t result = 0;
	int8_t* data = (int8_t*)(&result);

	for (int j = 0; j < 4; j++)
	{
		int8_t byte = value >> (j * 0x8);
		*data = reverseBits(byte);
		data++;
	}

	return result;
}

Vec4 
vCMeshShapes::loadVertexWeights(char*& data)
{
	Vec4 weights;
	uint16_t vtxDeltaZ = ReadUInt16(data);
	uint16_t vtxDeltaY = ReadUInt8(data);
	uint16_t vtxDeltaX = ReadUInt8(data);
	uint16_t unkValue  = ReadUInt16(data);
	weights.w		   = ReadUInt16(data); /* Defines vertex normal delta */

	/* Calculate X weight value */
	weights.x = m_tfmLowest.x + 8.0f * m_tfmDelta.x * vtxDeltaX;

	/* Calculate Z nibble values */
	int quotientZ, remainderZ;
	divmod(vtxDeltaZ, 1024, &quotientZ, &remainderZ);
	weights.z = m_tfmLowest.z + (remainderZ * m_tfmDelta.z);

	/* Calculate Y nibble values */
	int quotientY, remainderY;
	divmod(vtxDeltaY, 32, &quotientY, &remainderY);
	weights.y = m_tfmLowest.y + ( (quotientZ + (64.0f * remainderY) ) * m_tfmDelta.y);

	return weights;
}

void
vCMeshShapes::applyVertexWeight(const size_t vertexIndex, std::vector<float>& vertices, const Vec4& vertexWeights)
{
	size_t vectorPos = (vertexIndex * 3);
	auto& coordX = vertices[vectorPos];
	auto& coordY = vertices[vectorPos+1];
	auto& coordZ = vertices[vectorPos+2];

	coordX += vertexWeights.x;
	coordY += vertexWeights.y;
	coordZ += vertexWeights.z;
}

void
vCMeshShapes::getVertexWeights( const uint32_t& compression, char*& weightData, StBlendShape* targetShape, const int& index )
{
	uint32_t level = flip32bits(compression);

	/* Align size of data to vertex count */
	int32_t len = ((index + 0x20) >= m_numMorphVerts) ?
		(m_numMorphVerts)-index : 0x20;

	/* Collect and update all weighted vertex indices */
	for (int32_t j = 0; j < len; j++)
	{
		int32_t vtxIndex = index + j;
		bool isMorphed = (level >> (0x1F - j)) & 1;

		/* Apply vertex morph weights if value is defined */
		if (isMorphed){
			Vec4 weights = loadVertexWeights(weightData);
			applyVertexWeight(vtxIndex, targetShape->vertices, weights);
			targetShape->vtxMorphs.push_back(vtxIndex);
		}
	}
}

void
vCMeshShapes::getVertexDeltas(char* table, StBlendShape* targetShape )
{
	int index = 0;
	while (index < m_numMorphVerts)
	{
		uint32_t compressionLevel = ReadUInt32(m_data, true);
		char* weightsPtr 	      = table + (ReadUInt32(m_data) * uintptr_t(4));

		/* Collect and update all weighted vertices */
		getVertexWeights(compressionLevel, weightsPtr, targetShape, index);
		index += 0x20;
	}
}

void
vCMeshShapes::getMorphWeights() 
{
	/* Get base deltas and lowest x,y,z positions */
	vCMeshShapes::getTransformDeltas();
	uint32_t tableSize  = ReadUInt32(m_data);
	char* morphsTable   = m_data;

	for (int i = 0; i < m_numMorphs; i++){
		auto& blendshape = m_shapes->at(i);
		getVertexDeltas(morphsTable, &blendshape);
	}

	m_data = morphsTable + tableSize; // seek to end
}

void
vCMeshShapes::getMorphIds() 
{
	this->m_numMorphs = ReadUInt32(m_data);

	/* Aquire all blend shape id's */
	for (int i = 0; i < m_numMorphs; i++) {
		StBlendShape sBlendShape;
		int index = ReadUInt16(m_data);

		sBlendShape.name	 = m_stringTable.at(index);
		sBlendShape.vertices = m_mesh->vertices;
		m_shapes->push_back(sBlendShape);
	}

	/* Align stream */
	m_data = Data::roundPointerToNearest4(m_data);
}

void
vCMeshShapes::load() 
{
	if (!m_data) 
		return;

	/* Collect all blendshape names */
	this->getMorphIds();

	/* Collect all vertex weight + coordinates */
	if (m_numMorphs > 0)
		this->getMorphWeights();
}

