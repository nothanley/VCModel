#include <cstring>
#include <VCModel>
#pragma once

/* Free memory allocated during interface */
extern "C" __declspec(dllexport) void freeMemory_float32(float* data);

extern "C" __declspec(dllexport) void freeMemory_intArr(int* data);

extern "C" __declspec(dllexport) void freeSkinModel(void* pSkinModel);

extern "C" __declspec(dllexport) void freeMemory_charArrPtr(const char** data);

extern "C" __declspec(dllexport) void freeMemory_skinData(void* pSkinData);

/* Interface methods for accessing 'CSkinModel' object data */
extern "C" __declspec(dllexport) void* loadModelFile(const char* filePath);

extern "C" __declspec(dllexport) int getMeshTotal(void* pSkinModel);

extern "C" __declspec(dllexport) const char* getMaterialName(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const char* getMeshName(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getVertexData(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getMeshNormals(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumVerts(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getMeshSceneFlag(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getMeshMotionFlag(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumUvChannels(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const uint32_t* getMeshTriangleList(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumTriangles(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getMeshUvChannel(void* pSkinModel, const int meshIndex, const int channelIndex);

extern "C" __declspec(dllexport) int getNumBones(void* pSkinModel);

extern "C" __declspec(dllexport) const char* getBoneName(void* pSkinModel, const int boneIndex);

extern "C" __declspec(dllexport) const float* getBoneTransformMatrix(void* pSkinModel, const int boneIndex);

extern "C" __declspec(dllexport) int getBoneParentIndex(void* pSkinModel, const int boneIndex);

extern "C" __declspec(dllexport) void* getSkinData(void* pSkinModel, const int meshIndex);

extern "C" __declspec(dllexport) const char** getAllSkinGroups(void* pSkinData, int* numBones);

extern "C" __declspec(dllexport) const float* getAllJointWeights(void* pSkinData, const char* target, int* size);

extern "C" __declspec(dllexport) int getNumMeshVertexColors(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getMeshVertexColors(void* pSkinModel, const int meshIndex, const int setIndex, int* size);

extern "C" __declspec(dllexport) const char** getAllMeshMorphs(void* pSkinModel, const int meshIndex, int* numShapes);

extern "C" __declspec(dllexport) const float* getMeshBlendShape(void* pSkinModel, const int meshIndex, const int shapeIndex, int* size);

extern "C" __declspec(dllexport) const int* getVtxMorphIndices(void* pSkinModel, const int meshIndex, const int shapeIndex, int* size);

extern "C" __declspec(dllexport) const char** getAllFaceGroups(void* pSkinModel, const int meshIndex, int* size);

extern "C" __declspec(dllexport) void getMaterialFaceGroup(void* pSkinModel, const int meshIndex, const int groupIndex, int* faceBegin, int* faceSize);


