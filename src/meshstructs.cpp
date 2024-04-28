#include "meshstructs.h"
#include "BinaryIO.h"
#include <glm/gtx/euler_angles.hpp>
#include "meshshapes_serialize.h"

using namespace BinaryIO;
//using namespace MeshSerializer;

inline
glm::mat4 dot_4x4(const glm::mat4x4& a, const glm::mat4x4& b)
{
	glm::mat4 result;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			float num = 0;

			for (int k = 0; k < 4; k++)
				num += a[i][k] * b[k][j];

			result[i][j] = num;
		}

	return result;
}

void RigBone::set_parent(RigBone* pParentBone)
{
	this->parent = pParentBone;
	this->parent->children.push_back(this);

	// update world space transform
	this->matrix_world = dot_4x4(this->matrix_local, parent->matrix_world);
}

void RigBone::set_transform(float* matrices, const bool& reorder_matrix) {
	auto& transform = this->matrix_local;

	/* Interpret the matrices in order,
	or arrange the matrix with respect to Blender's 'mathutils' module */
	if (reorder_matrix) {
		transform[0] = glm::vec4{ matrices[0],  matrices[4],  matrices[8],  matrices[12] };
		transform[1] = glm::vec4{ matrices[1],  matrices[5],  matrices[9],  matrices[13] };
		transform[2] = glm::vec4{ matrices[2],  matrices[6],  matrices[10], matrices[14] };
		transform[3] = glm::vec4{ matrices[3],  matrices[7],  matrices[11], matrices[15] };
	}
	else {
		transform[0] = glm::vec4{ matrices[0],  matrices[1],  matrices[2],  matrices[3] };
		transform[1] = glm::vec4{ matrices[4],  matrices[5],  matrices[6],  matrices[7] };
		transform[2] = glm::vec4{ matrices[8],  matrices[9],  matrices[10], matrices[11] };
		transform[3] = glm::vec4{ matrices[12], matrices[13], matrices[14], matrices[15] };
	}
}

void Mesh::generateAABBs()
{
	/* Validate coordinate vector */
	const auto& coords = this->vertices;
	if (coords.size() < 3) return;

	// Create default bounding box
	BoundingBox& box = this->bounds;
	box = BoundingBox{ /* Max Coords */ coords[0], coords[1], coords[2],
		/* Min Coords */ coords[0], coords[1], coords[2] };

	// Iterate and find highest/lowest position coord
	int numCoords = coords.size();
	for (int i = 0; i < numCoords; i += 3)
	{
		Vec3 vertex{ coords[i], coords[i + 1], coords[i + 2] };

		box.maxX = (vertex.x > box.maxX) ? vertex.x : box.maxX;
		box.maxY = (vertex.y > box.maxY) ? vertex.y : box.maxY;
		box.maxZ = (vertex.z > box.maxZ) ? vertex.z : box.maxZ;

		box.minX = (vertex.x < box.minX) ? vertex.x : box.minX;
		box.minY = (vertex.y < box.minY) ? vertex.y : box.minY;
		box.minZ = (vertex.z < box.minZ) ? vertex.z : box.minZ;
	}
}


void Mesh::flipNormals()
{
	for (auto& tri : triangles)
		tri = Triangle{ tri[1], tri[0], tri[2] };
}

void Mesh::convertSplitNorms()
{
	int array_size = (normals.size() == vertices.size()) ? 3 : 4;
	std::vector<float> data;

	for (int i = 0; i < numVerts; i++)
	{
		int index = (i * array_size);
		data.push_back(normals.at(index + 0));
		data.push_back(normals.at(index + 1));
		data.push_back(normals.at(index + 2));
	}

	this->normals = data;
}

void Mesh::translateUVs(const int& index)
{
	if (index > uvs.size())
		return;

	auto& map = this->uvs.at(index).map;
	/* Flip Y axis and translate up by one unit */
	for (int i = 0; i < map.size(); i += 2) {
		map[i + 1] = -(map[i + 1] - 1.0f);
	}
}

inline void
LoadVertexSkin(const Skin* skin, BlendWeight& skinVertex,
	const std::vector<std::string>& stringTable, int& begin, const int& numWeights)
{
	/* Iterate through all specified weights for current vertex */
	for (int j = 0; j < numWeights; j++)
	{
		int index = skin->indices[begin];
		float influence = skin->weights[begin];

		std::string boneName = stringTable.at(index);
		skinVertex.bones.push_back(boneName);
		skinVertex.weights.push_back(influence);

		begin++; // Update skin pointer to next index
	}
}

std::vector<BlendWeight>*
Skin::unpack(const std::vector<std::string>& stringTable)
{
	int numVerts = this->weights.size() / numWeights;
	std::vector<BlendWeight>* skinData = new std::vector<BlendWeight>;
	skinData->resize(numVerts);

	/* Iterate through all skin vertices */
	for (int i = 0; i < numVerts; i++)
	{
		BlendWeight& skinVtx = skinData->at(i);
		int skinPtr = (i * numWeights);
		LoadVertexSkin(this, skinVtx, stringTable, skinPtr, numWeights);
	}

	return skinData;
}

Vec2 UVMap::texcoord(const int index) const
{
	int offset = (index * 2);
	return Vec2{ map[offset],  map[offset + 1]};
}

Vec3 Mesh::vertex(const int index) const 
{
	int offset = (index * 3);
	return Vec3{ vertices[offset],  vertices[offset + 1],  vertices[offset + 2] };
}

Vec3 StBlendShape::vertex(const int index) const 
{
	int offset = (index * 3);
	return Vec3{ vertices[offset],  vertices[offset + 1],  vertices[offset + 2] };
}

Vec3 Mesh::normal(const int index) const
{
	int array_width = normals.size() / numVerts;
	int offset = (index * array_width);
	return Vec3{ normals[offset],  normals[offset + 1],  normals[offset + 2] };
}



