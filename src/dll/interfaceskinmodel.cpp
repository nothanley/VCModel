#include "interfaceskinmodel.h"
#include <VCModel>
#include <vector>

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

int getNumUvChannels(void* pSkinModel, const int index) 
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return 0;

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->texcoords.size();
}

const float* getMeshUvChannel(void* pSkinModel, const int meshIndex, const int channelIndex)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return 0;

    /* Load mesh */
    auto mesh = model->getMeshes().at(meshIndex);
    auto uvs = &mesh->texcoords;

    if (channelIndex > uvs->size())
        return nullptr;

    /* Extrapolate uv channel */
    mesh->translateUVs(channelIndex);

    auto& channel = uvs->at(channelIndex).map;
    return channel.data();
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
    mesh->flipNormals();

    /* Cast triangle vector to index list */  
    /* todo: this is hacky, if we alter the triangle struct we cannot use a cast */
    auto tris = reinterpret_cast< std::vector<uint32_t>* >( &mesh->triangles );

    return tris->data();
}


const float* getMeshNormals(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    mesh->convertSplitNorms();

    /* Rearrange normal data */
    return mesh->normals.data();
}

int getNumBones(void* pSkinModel) 
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model)
        return 0;

    return model->getNumBones();
}


const char* getBoneName(void* pSkinModel, const int boneIndex) 
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || boneIndex > model->getNumBones())
        return nullptr;

    /* Load mesh */
    auto bone = model->getBones().at(boneIndex);

    /* Check valid bone entry */
    if (!bone) 
        return nullptr;

    //printf("\nLoading Bone: %s\n", bone->name.c_str());
    return bone->name.c_str();
}

int getBoneParentIndex(void* pSkinModel, const int boneIndex)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || boneIndex > model->getNumBones())
        return -1;

    /* Load skeleton */
    auto  skeleton = model->getBones();
    auto& parent   = skeleton.at(boneIndex)->parent;

    if (parent == nullptr)
        return -1;

    return parent->index;
}

const float* getGlmMatToFloatPtr(const glm::mat4& mat) {
    // Allocate memory for the matrix data
    float* transform = new float[16];

    // Load matrix data into the array
    transform[0]  = mat[0].x;  
    transform[1]  = mat[0].y; 
    transform[2]  = mat[0].z; 
    transform[3]  = mat[0].w;

    transform[4]  = mat[1].x;  
    transform[5]  = mat[1].y;  
    transform[6]  = mat[1].z;  
    transform[7]  = mat[1].w;

    transform[8]  = mat[2].x;  
    transform[9]  = mat[2].y; 
    transform[10] = mat[2].z;  
    transform[11] = mat[2].w;

    transform[12] = mat[3].x;  
    transform[13] = mat[3].y; 
    transform[14] = mat[3].z; 
    transform[15] = mat[3].w;

    return transform;
}

const float* getBoneTransformMatrix(void* pSkinModel, const int boneIndex)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || boneIndex > model->getNumBones())
        return nullptr;

    /* Load bone matrices */
    auto bone = model->getBones().at(boneIndex);
    return getGlmMatToFloatPtr(bone->matrix);
}

inline bool stringExistsInVector(const std::vector<std::string*>& vec, const std::string* target) 
{
    for (auto& string : vec) 
        if (*string == *target)
            return true;

    return false;
}

void freeMemory_float32(float* set) 
{
    if (!set) return;
    delete[] set;
}

void freeMemory_charArrPtr(const char** set)
{
    if (!set) return;
    delete[] set;
}

void freeMemory_skinData(void* pSkinData)
{
    // Convert void pointer back to CSkinModel pointer
    std::vector<BlendWeight>* skin = static_cast<std::vector<BlendWeight>*>(pSkinData);
    if (!skin)
        return;

    delete skin;
}

void* getSkinData(void* pSkinModel, const int meshIndex)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh   = model->getMeshes().at(meshIndex);
    auto sTable = model->getStringTable();

    if (mesh->skin.numWeights == 0)
        return nullptr;

    return mesh->skin.unpack(*sTable);
}

const char** getAllSkinGroups(void* pSkinData, int* numBones)
{
    // Convert void pointer back to CSkinModel pointer
    std::vector<BlendWeight>* skin = static_cast<std::vector<BlendWeight>*>(pSkinData);
    if (!skin)
        return nullptr;
    
    // Iterate through skin data and get a string list of all affected groups...
    std::vector<std::string*> bones;

    for (auto& bw : *skin) {
        for (auto& bone : bw.bones)
            if (!stringExistsInVector(bones, &bone))
            {
                bones.push_back(&bone);
            }
    }

    // Convert std::vector<std::string> to array of char pointers
    *numBones = static_cast<int>(bones.size());
    const char** arr = new const char* [*numBones];
    for (size_t i = 0; i < *numBones; ++i) {
        arr[i] = bones[i]->c_str();
    }

    return arr;
 }

const float* getAllJointWeights(void* pSkinData, const char* target, int* size)
{
    // Convert void pointer back to CSkinModel pointer
    std::vector<BlendWeight>* skin = static_cast<std::vector<BlendWeight>*>(pSkinData);
    if (!skin)
        return nullptr;

    int numVerts = skin->size();
    float* vtxWeights = new float[numVerts];

    // Iterate through skin data and get a weight list for all verts of specified bone..
    for (int i = 0; i < numVerts; i++) 
    {
        auto& bw = skin->at(i);
        int numVtxBones = bw.bones.size();
        vtxWeights[i] = 0.0f;

        for (int j = 0; j < numVtxBones; j++)
            if (bw.bones.at(j) == target) {
                vtxWeights[i] = bw.weights.at(j);
                break;
            }
    }

    *size = numVerts;
    return vtxWeights;
}


int getNumMeshVertexColors(void* pSkinModel, const int index)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return 0;
     
    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->colors.size();
}

const float*
getMeshVertexColors(void* pSkinModel, const int meshIndex, const int setIndex, int* size) 
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(meshIndex);
    if (setIndex > mesh->colors.size())
        return nullptr;

    auto& colorSet = mesh->colors.at(setIndex);
    *size = colorSet.map.size();
    return colorSet.map.data();
}

