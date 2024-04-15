#include <string>
#include <vector>
#include "databuffers.h"
#pragma once

static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

class vCMeshShapes
{
public:
	vCMeshShapes(char*& data, const std::vector<std::string>& stringTable, Mesh* mesh);

public:
	void load();
	std::vector<StBlendShape>* getBlendShapes() { return m_shapes; }

private:
	inline void getTransformDeltas();
	void getMorphIds();
	void getMorphWeights();

private:
	void getVertexDeltas(char* tablePtr, StBlendShape* targetShape);
	void getVertexWeights(const uint32_t& compression, char*& weightData, StBlendShape* targetShape, const int& index);

private:
	inline uint8_t reverseBits(uint8_t n);
	inline uint32_t flip32bits(uint32_t value);
	inline void divmod(int a, int b, int* quotient, int* remainder);
	Vec4 loadVertexWeights(char*& data);
	void applyVertexWeight(const size_t vertexIndex, std::vector<float>& vertices, const Vec4& vertexWeights);

private:
	char* m_data;
	Mesh* m_mesh;
	uint32_t m_numMorphs = 0;
	uint32_t m_numMorphVerts = 0;
	std::vector<std::string> m_stringTable;
	std::vector<StBlendShape>* m_shapes;
	Vec3 m_tfmDelta;
	Vec3 m_tfmLowest;
};