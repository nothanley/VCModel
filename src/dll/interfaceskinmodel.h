#include <cstring>
#include <VCModel>
#pragma once

extern "C" __declspec(dllexport) void* loadModelFile(const char* filePath);

extern "C" __declspec(dllexport) void freeSkinModel(void* pSkinModel);

extern "C" __declspec(dllexport) int getMeshTotal(void* pSkinModel);

extern "C" __declspec(dllexport) const char* getMeshName(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getVertexData(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const float* getMeshNormals(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumVerts(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) const uint32_t* getMeshTriangleList(void* pSkinModel, const int index);

extern "C" __declspec(dllexport) int getNumTriangles(void* pSkinModel, const int index);

