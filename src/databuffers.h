#include <vector>
#include <string>
#include <iostream>
#pragma once

struct Vec3 {
	float x, y, z;
};

struct Vec4 {
	float x, y, z, w;
};

struct RigBone {
	Vec3 translation;
	Vec4 quaternion;
	std::string name;
	RigBone* parent = nullptr;
	std::vector<RigBone*> children;
};

struct Material {
	std::string name;
};

struct VertexColorSet {
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

struct Mesh {
	std::string name;
	Material material;
	BoundingBox bounds;

	int sceneFlag,
		motionFlag,
		numVerts;

	Skin skin;
	std::vector<float> vertices, normals, texcoords;
	std::vector<float> binormals, tangents;
	std::vector<VertexColorSet> colors;
	std::vector<Triangle> triangles;
};

struct MeshBuffer {
	std::string property, format, type;
	char* stream;
};

namespace VCModel {

	class CDataBuffer
	{
	public:
		static 
			void getStringTable(char* stringBuffer, const uintptr_t& blockPos, std::vector<std::string>& stringTable);
		
		static 
			void getModelBones(char* buffer, const uintptr_t& size, std::vector<RigBone*>& bones, const std::vector<std::string>& stringTable, const int& containerType);
		
		static
			void getModelBones_Ver_2_8(char* buffer, const uintptr_t& size, std::vector<RigBone*>& bones,
				const std::vector<std::string>& stringTable);

		static 
			void getMaterials(char* buffer, const uintptr_t& size, std::vector<Material>& materials, const std::vector<std::string>& stringTable);
		static 
			void getMeshes(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, const std::vector<std::string>& stringTable);
		static
			void getLods(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, const std::vector<Material>& mtlTable, const std::vector<std::string>& strings);
	};

}