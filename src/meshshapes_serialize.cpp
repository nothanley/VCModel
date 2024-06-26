#include "meshshapes_serialize.h"
#include "meshbuffers.h"
#include "BinaryIO.h"
#include "winsock.h"
#undef min
#undef max

using namespace BinaryIO;
using namespace MeshSerializer;


Vec3 vCMeshShapeSerial::getVertex(int index, const std::vector<float>& coords) {
	return Vec3{ coords[index],  coords[index + 1],  coords[index + 2] };
}

void vCMeshShapeSerial::updateShapeVtxIndexBuffer(std::stringstream& stream, uint32_t& indexMask, uint32_t& offset, uint8_t& size)
{
	WriteUInt32(stream, indexMask);
	WriteUInt32(stream, offset / 4);

	/* Reset mask and update stream's begin offset */
	offset += (size * 8);
	indexMask = 0;
	size = 0;
}


void vCMeshShapeSerial::updateMinMaxDeltas(Matrix3& deltaMat, Mesh* mesh, const StBlendShape& shape)
{
	Vec3& mins = deltaMat.x;
	Vec3& maxs = deltaMat.y;
	int numCoords = shape.vertices.size();

	for (int i = 0; i < numCoords; i += 3) {
		Vec3 originalVertex   = getVertex(i, mesh->vertices);
		Vec3 blendShapeVertex = getVertex(i, shape.vertices);

		Vec3 delta{ blendShapeVertex.x - originalVertex.x,
					blendShapeVertex.y - originalVertex.y,
					blendShapeVertex.z - originalVertex.z };

		/* Update minmax vectors */
		Vec3::min(mins, delta, mins);
		Vec3::max(maxs, delta, maxs);
	}
};

void vCMeshShapeSerial::calculatePrecisionValues(Matrix3& delta_matrix)
{
	Vec3& overall_min = delta_matrix.x;
	Vec3& overall_max = delta_matrix.y;
	Vec3& precision	  = delta_matrix.z;

	precision.x = std::abs(overall_max.x - overall_min.x) / 2040.0f;
	precision.y = std::abs(overall_max.y - overall_min.y) / 1984.0f;
	precision.z = std::abs(overall_max.z - overall_min.z) / 1023.0f;
}

Matrix3 vCMeshShapeSerial::getBlendShapePrecisionMatrix(Mesh* mesh)
{
	/* Compare all object vertices and collect the greatest min/max deltas on each axis */
	Matrix3 delta_matrix{};
	for (auto& shape : mesh->blendshapes) {
		updateMinMaxDeltas(delta_matrix, mesh, shape);
	}

	/* Compare all delta values and extrapolate precision based on XYZ compression width */
	calculatePrecisionValues(delta_matrix);
	return delta_matrix;
}

void vCMeshShapeSerial::writeDeltaMatrix(std::stringstream& stream, Matrix3& delta_matrix)
{
	Vec3& deltaPrecision = delta_matrix.z;
	Vec3& deltaMinimum = delta_matrix.x;

	WriteFloat(stream, deltaPrecision.x);
	WriteFloat(stream, deltaPrecision.y);
	WriteFloat(stream, deltaPrecision.z);

	WriteFloat(stream, deltaMinimum.x);
	WriteFloat(stream, deltaMinimum.y);
	WriteFloat(stream, deltaMinimum.z);
}


void vCMeshShapeSerial::setBit(uint32_t& value, const int8_t index, const bool sign){
	value |= uint32_t(sign << index);
}

static int roundUp(int numToRound, int multiple) {
	auto remainder = numToRound % multiple;
	return (numToRound + multiple - remainder);
}

void vCMeshShapeSerial::compressVertexDelta(Vec3& delta, const Vec3& lowest, const Vec3& precision)
{
	int quotient, remainder_y, remainder_z;

	/* Compress X vertex delta */
	delta.handleNaN();
	delta.x = (delta.x - lowest.x) / (precision.x * 8);

	/* Compress Y vertex delta */
	delta.y = (delta.y - lowest.y) / precision.y;
	::vCMeshShapes::divmod(delta.y, 63, &quotient, &remainder_y);
	delta.y = quotient;

	/* Compress Z vertex delta */
	delta.z = (delta.z - lowest.z) / precision.z;
	::vCMeshShapes::divmod(delta.z, 1024, &quotient, &remainder_z);
	delta.z = remainder_z + (std::fmin(remainder_y, 63) * 1024);
}

void vCMeshShapeSerial::vCMeshShapeSerial::updateShapeVtxWeightBuffer(std::stringstream& stream, const Matrix3& deltaMatrix, 
	const Vec3& original_vertex, const Vec3& blendshape_vertex)
{
	Vec3 delta = blendshape_vertex - original_vertex;
	compressVertexDelta(delta, deltaMatrix.x, deltaMatrix.z);

	WriteSInt16( stream, delta.z );
	WriteSInt8 ( stream, delta.y );
	WriteSInt8 ( stream, delta.x );
	WriteUInt32( stream, 0		 ); // normal delta
}

