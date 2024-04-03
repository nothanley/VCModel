#include "interfaceskinmodel.h"
#include <VCModel>

void* loadModelFile(const char* filePath)
{
    try {
        CModelContainer mdlFile(filePath);
        return mdlFile.getModel();
    }
    catch (...) {
        printf("Failed to read user model file.\n");
    }

    return nullptr;
}


void freeSkinModel(void* pSkinModel) 
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model) return;

    delete model;
}

int getMeshTotal(void* pSkinModel) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model)
        return 0;

    /* accumulate mesh total */
    return model->getNumMeshes();
}

const float* getVertexData(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->vertices.data();
}

int getNumVerts(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return 0;

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->numVerts;
}

const char* getMeshName(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return "";

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->material.name.c_str();
}

const char* getMeshTriangles(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return "";

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->material.name.c_str();
}

int getNumTriangles(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return 0;

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->triangles.size();
}

const uint32_t* getMeshTriangleList(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);   

    /* Cast triangle vector to index list */  
    /* todo: this is hacky, if we alter the triangle struct we cannot use a cast */
    auto tris = reinterpret_cast< std::vector<uint32_t>* >( &mesh->triangles );

    return tris->data();
}


