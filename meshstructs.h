#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#pragma once

struct Vec3 {
	float x, y, z;

	void pack_values(float ceiling) {
		x = (x > ceiling) ? ceiling : x;
		y = (x > ceiling) ? ceiling : y;
		z = (x > ceiling) ? ceiling : z;

		float floor = -ceiling;
		x = (x < floor) ? floor : x;
		y = (x < floor) ? floor : y;
		z = (x < floor) ? floor : z;
	}

	void operator*=(float value) {
		x *= value;
		y *= value;
		z *= value;
	}

	Vec3 operator-(const Vec3& other) const {
		return Vec3{ x - other.x, y - other.y, z - other.z };
	}

	bool operator==(const Vec3& other) const {
		return (x == other.x) && (y == other.y) && (z == other.z);
	}

	bool operator!=(const Vec3& other) const {
		return (x != other.x) || (y != other.y) || (z != other.z);
	}

	static void min(Vec3& result, const Vec3& a, const Vec3& b)
	{
		result.x = std::min(a.x, b.x);
		result.y = std::min(a.y, b.y);
		result.z = std::min(a.z, b.z);
	};

	static void max(Vec3& result, const Vec3& a, const Vec3& b)
	{
		result.x = std::max(a.x, b.x);
		result.y = std::max(a.y, b.y);
		result.z = std::max(a.z, b.z);
	};

	void handleNaN(float value = 0.00001f) {
		if (std::isnan(x)) x = value;
		if (std::isnan(y)) y = value;
		if (std::isnan(z)) z = value;
	}

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
	std::string name;
	RigBone* parent = nullptr;
	std::vector<RigBone*> children;

	glm::mat4 matrix_local; // Parent space transform
	glm::mat4 matrix_world; // World space transform
	void set_transform(float* matrices, const bool& reorder_matrix);
	void set_parent(RigBone* parent);
};

struct Material
{
	std::string name;
};

struct VertexColorSet
{
	std::string name;
	std::vector<float> map;
};

struct BoundingBox
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float radius;
};

struct BlendWeight {
	std::vector<std::string> bones;
	std::vector<float> weights;
};

struct StBlendShape {
	std::string name;
	std::vector<float> vertices;
	std::vector<int>   vtxMorphs;
};

struct Skin {
	int numWeights = 0;
	std::vector<float> weights, indices;

	/* Unpacks all weights and indices into a bw struct vector */
	std::vector<BlendWeight>* unpack(
		const std::vector<std::string>& stringTable);
};

struct Triangle {
	uint32_t x, y, z;
};

struct UVMap {
	std::string name;
	float baseU, baseV;
	std::vector<float> map;
};

struct FaceGroup
{
	Material material;
	int faceBegin;
	int numTriangles;
};

struct Mesh
{
	std::string name;
	std::string definition;
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
	std::vector<StBlendShape> blendshapes;
	std::vector<FaceGroup> groups;

	/* Flips all mesh triangle faces inside out. */
	void flipNormals();

	/* Re-arrange normals for blender import/interface */
	void convertSplitNorms();

	/* Translates and aligns UV map to Blender/MAX 3D space*/
	void translateUVs(const int& index);

	/* Generates mesh's axis aligned boundary box */
	void generateAABBs();

	/* Calculate tangent + binormal vertex data */
	void generateTangentsBinormals();
};

struct MeshBuffer {
	std::string property, format, type;
	char* stream;
};
