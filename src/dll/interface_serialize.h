#include <cstring>
#include <VCModel>
#include <chrono>
#pragma once

extern "C" __declspec(dllexport)
void freeMesh(void* pMesh)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh)
		return;

	delete mesh;
}

extern "C" __declspec(dllexport) 
void* getNewSkinModel()
{
	CSkinModel* model = new CSkinModel();
	return model;
}

extern "C" __declspec(dllexport)
void linkMeshToModel(void* pSkinModel, void* pMesh)
{
	// Convert void pointer back to CSkinModel pointer
	CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	model->push_mesh(mesh);
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
void calculateMeshTangents(void* pMesh)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	mesh->calculateTangentsBinormals();
	return;
}

extern "C" __declspec(dllexport)
void setMeshData(void* pMesh, float* position, int* indexList, int numVerts, int numFaces)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	/* Populate position coords - (+X-Z+Y)*/
	mesh->vertices.resize(numVerts);
	for (int i = 0; i < numVerts; i+=3) {
		mesh->vertices[i]	=  position[i];
		mesh->vertices[i+1] = -position[i+2];
		mesh->vertices[i+2] =  position[i+1];
	}

	/* Set triangle indices */
	numFaces /= 3;
	mesh->triangles.resize(numFaces);
	for (int i = 0; i < numFaces; i++) 
	{
		Triangle& face = mesh->triangles.at(i);
		size_t index = (i * 3);

		face.x = indexList[index+1];
		face.y = indexList[index];
		face.z = indexList[index+2];
	}

	/* Update counts */
	mesh->numVerts = mesh->vertices.size() / 3;
	mesh->generateAABBs();

	/* Setup mesh default mtl */
	FaceGroup faceMat{ mesh->material, 0, numFaces};
	mesh->groups.push_back(faceMat);
}

extern "C" __declspec(dllexport)
void addBlendShape(void* pMesh, const char* name, float* coords, int size)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	/* Validate coordinate array*/
	if (size != mesh->vertices.size())
		return;

	/* Push shape key to mesh struct */
	StBlendShape shape;
	shape.name = name;
	shape.vertices.resize(size);
	for (int i = 0; i < size; i += 3) {
		/* Populate position coords - (+X-Z+Y)*/
		shape.vertices[i]	  =  coords[i];
		shape.vertices[i + 1] =  -coords[i+2];
		shape.vertices[i + 2] =  coords[i+1];
	}

	mesh->blendshapes.push_back(shape);
}

extern "C" __declspec(dllexport)
void setMeshNormals(void* pMesh, float* normals, int size)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	int numNorms = size / 3;
	if (numNorms != mesh->numVerts) {
		printf("[CSkinModel] Vertex normal count mismatch.");
		throw std::runtime_error("");
	}

	/* Populate vertex normals -- seems slow */
	for (int i = 0; i < size; i+=3) {
		mesh->normals.push_back(normals[i]);
		mesh->normals.push_back(-normals[i+2]);
		mesh->normals.push_back(normals[i+1]);
		mesh->normals.push_back(0.0f);
	}
}

extern "C" __declspec(dllexport)
void addUvMap(void* pMesh, float* uvs, int size)
{
	Mesh* mesh = static_cast<Mesh*>(pMesh);
	if (!mesh) return;

	/* Create a new uv channel */
	UVMap channel;
	channel.map.resize(size);
	for (int i = 0; i < size; i+=2) {
		channel.map.at(i)	= uvs[i];
		channel.map.at(i+1) = -(-1.0 + uvs[i+1]);
	}

	mesh->uvs.push_back(channel);
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
	if (!mesh)
		return;

	//printf("\n[CSkinModel] Populating Skin with limit: %d\n", numWeightsPerVtx);
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

inline int
findBone(const std::vector<RigBone*>& bones, const std::string& target) {
	int numBones = bones.size();

	for (int i = 0; i < numBones; i++) {
		auto& bone = bones.at(i);

		if (bone->name == target)
			return i;
	}

	return -1;
}

extern "C" __declspec(dllexport)
void setNewModelBone(void* pSkinModel, const char* name, float* matrices,
	const int index, const char* parent, bool reorder_matrix=true)
{
	// Convert void pointer back to CSkinModel pointer
	CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
	if (!model) return;

	const auto& bones  = model->getBones();
	RigBone* bone = new RigBone;
	bone->index = index;
	bone->name  = name;
	bone->set_transform(matrices, reorder_matrix);
	model->push_bone(bone);

	/* Update parent hierarchy */
	int parentIndex = (parent == "") ? -1 : findBone(bones, parent);
	if (parentIndex != -1) {
		RigBone* parent = bones.at(parentIndex);
		bone->set_parent(parent);
	}

}



extern "C" __declspec(dllexport)
void saveModelToFile(void* pSkinModel, const char* savePath, int compile_target)
{
	// Convert void pointer back to CSkinModel pointer
	CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
	if (!model) return;

	try {
		auto start = std::chrono::high_resolution_clock::now();

		switch (compile_target)
		{
			case 0x28:{/* Save MDL format v2.8*/
				CModelSerializer serializer(model);
				serializer.save(savePath);
				printf("\n[CSkinModel] MDL v2.8 file saved to: \"%s\"\n", savePath); }
				break;
			default:
				break;
		}

		// Get the current time after executing the function
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		float seconds = duration.count() / 1'000'000.0f;
		printf("\n[CSkinModel] Serialization Time: %f seconds.\n", seconds);
	}
	catch (...) {
		printf("\n[CSkinModel] Failed to save model file.");
	}
	
}