/* Extends mesh shapes class with serialization and compression functionality */
#include "meshshapes.h"
#pragma once

class vCMeshShapeSerial : public vCMeshShapes
{
public:
	static Matrix3 getBlendShapePrecisionMatrix(Mesh* mesh);

	static Vec3 getVertex(int index, const std::vector<float>& coords);

	static void updateMinMaxDeltas(Matrix3& deltaMat, Mesh* mesh, const StBlendShape& shape);

	static void calculatePrecisionValues(Matrix3& delta_matrix);

	static void writeDeltaMatrix(std::stringstream& stream, Matrix3& delta_matrix);

	static void setBit(uint32_t& value, const int8_t index, const bool sign = true);

	static void updateShapeVtxIndexBuffer(std::stringstream& stream, uint32_t& indexMask, uint32_t& offset, uint8_t& size);

	static void compressVertexDelta(Vec3& delta, const Vec3& lowest, const Vec3& precision);

	static void updateShapeVtxWeightBuffer(std::stringstream& stream, const Matrix3& deltaMatrix, const Vec3& original_vertex, const Vec3& blendshape_vertex);
};
