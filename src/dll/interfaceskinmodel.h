#include <cstring>
#include <VCModel>
#pragma once

/* Free memory allocated during interface */
extern "C" __declspec(dllexport) void freeMemory_float32(float* data);

extern "C" __declspec(dllexport) void freeSkinModel(void* pSkinModel);


/* Interface methods for accessing 'CSkinModel' object data */

extern "C" __declspec(dllexport) void* loadModelFile(const char* filePath);

extern "C" __declspec(dllexport) int getMeshTotal(void* pSkinModel);

extern "C" __declspec(dllexport) const char* getMeshName(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getVertexData(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getMeshNormals(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumVerts(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumUvChannels(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const uint32_t* getMeshTriangleList(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumTriangles(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getMeshUvChannel(void* pSkinModel, const int meshIndex, const int channelIndex);

extern "C" __declspec(dllexport) int getNumBones(void* pSkinModel);

extern "C" __declspec(dllexport) const char* getBoneName(void* pSkinModel, const int boneIndex);

extern "C" __declspec(dllexport) const float* getBoneTransformMatrix(void* pSkinModel, const int boneIndex);

extern "C" __declspec(dllexport) int getBoneParentIndex(void* pSkinModel, const int boneIndex);

extern "C" __declspec(dllexport) void* getMeshSkinData(void* pSkinModel, const int meshIndex);






