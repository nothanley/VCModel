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
	if (!mesh)
		return;

	mesh->name			= meshName;
	mesh->material.name = mtlName;
	return;
}


