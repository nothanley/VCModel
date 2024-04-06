#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#pragma once

struct Vec3 {
	float x, y, z;
};

struct Vec4 {
	float x, y, z, w;
};

struct Matrix3 {
	Vec3 x, y, z;
};

struct Matrix4 {
	Vec4 x, y, z, w;
};

struct RigBone 
{
	int16_t index;
	glm::mat4 matrix;
	std::string name;
	RigBone* parent = nullptr;
	std::vector<RigBone*> children;
};

struct Material 
{
	std::string name;
};

struct VertexColorSet 
{
	std::vector<float> data;
};

struct BoundingBox
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float radius;
};

struct Skin {
	int numWeights = 0;
	std::vector<float> blendweights, blendindices;
};

struct Triangle {
	uint32_t x, y, z;
};

struct UVMap {
	std::vector<float> map;
};

struct Mesh {
	std::string name;
	Material material;
	BoundingBox bounds;

	int sceneFlag,
		motionFlag,
		numVerts;

	Skin skin;
	std::vector<float> vertices, normals;
	std::vector<float> binormals, tangents;
	std::vector<VertexColorSet> colors;
	std::vector<Triangle> triangles;
	std::vector<UVMap> texcoords;

	/* Flips all mesh triangle faces inside out. */
	void flipNormals();

	/* Re-arrange normals for blender import/interface */
	void convertSplitNorms();

	/* Translates and aligns UV map to Blender/MAX 3D space*/
	void translateUVs(const int& index);
};

struct MeshBuffer {
	std::string property, format, type;
	char* stream;
};

class CDataBuffer
{
public:
	static void getStringTable(char* stringBuffer, std::vector<std::string>& stringTable);

	static void getModelBones_2_5(char* buffer, const uintptr_t& size, std::vector<RigBone*>& bones, const std::vector<std::string>& stringTable);

	static void getModelBones_2_8(char* buffer, const uintptr_t& size, std::vector<RigBone*>& bones, const std::vector<std::string>& stringTable);

	static void getMaterials(char* buffer, const uintptr_t& size, std::vector<Material>& materials, const std::vector<std::string>& stringTable);

	static void getMeshes(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, const std::vector<std::string>& stringTable);

	static void getLods(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, const std::vector<Material>& mtlTable, const std::vector<std::string>& strings);

};