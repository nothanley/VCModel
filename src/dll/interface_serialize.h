#include <cstring>
#include <VCModel>
#pragma once

extern "C" __declspec(dllexport) 
void* getNewVcModel(int* version)
{
	CSkinModel* model = new CSkinModel(version);
	return model;
}

extern "C" __declspec(dllexport)
void freeMesh(void* pMesh)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh)
		return;

	delete mesh;
	return;
}

extern "C" __declspec(dllexport)
void* getNewMesh()
{
	Mesh* mesh = new Mesh;
	return mesh;
}

extern "C" __declspec(dllexport)
void setMeshNameInfo(void* pMesh, const char* meshName, const char* mtlName)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	mesh->name			= meshName;
	mesh->material.name = mtlName;
	return;
}


extern "C" __declspec(dllexport)
void setMeshData(void* pMesh, float* position, int* indexList, int numVerts, int numFaces)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	/* Populate position coords*/
	mesh->vertices.resize(numVerts);
	for (int i = 0; i < numVerts; i++) {
		mesh->vertices[i] = position[i];
	}

	/* Set triangle indices */
	numFaces /= 3;
	mesh->triangles.resize(numFaces);
	for (int i = 0; i < numFaces; i++) 
	{
		Triangle& face = mesh->triangles.at(i);
		size_t index = (i * 3);

		face.x = indexList[index];
		face.y = indexList[index+1];
		face.z = indexList[index+2];
	}
}

extern "C" __declspec(dllexport)
void setMeshNormals(void* pMesh, float* normals, int size)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	/* Populate vertex normals*/
	mesh->normals.resize(size);
	for (int i = 0; i < size; i++) {
		mesh->normals[i] = normals[i];
	}
}

extern "C" __declspec(dllexport)
void addUvMap(void* pMesh, float* texcoords, int size)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	/* Create a new uv channel */
	UVMap channel;

	channel.map.resize(size);
	for (int i = 0; i < size; i++) {
		channel.map.at(i) = texcoords[i];
	}

	mesh->texcoords.push_back(channel);
}


extern "C" __declspec(dllexport)
void addMeshColorMap(void* pMesh, float* colors, int size)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	/* Create a new vertex color channel */
	VertexColorSet channel;

	channel.map.resize(size);
	for (int i = 0; i < size; i++) {
		channel.map.at(i) = colors[i];
	}

	mesh->colors.push_back(channel);
	return;
}


extern "C" __declspec(dllexport)
void setMeshSkinData(void* pMesh, int* indices, float* weights, int size, int numWeightsPerVtx)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	auto& blendindices = mesh->skin.indices;
	auto& blendweights = mesh->skin.weights;
	mesh->skin.numWeights = numWeightsPerVtx;

	blendindices.resize(size);
	for (int i = 0; i < size; i++) {
		blendindices.at(i) = indices[i];
	}

	blendweights.resize(size);
	for (int i = 0; i < size; i++) {
		blendweights.at(i) = weights[i];
	}
}


