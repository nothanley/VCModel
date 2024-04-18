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


const char* getMaterialName(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return "";

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->material.name.c_str();
}


const char* getMeshName(void* pSkinModel, const int index) {
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || index > model->getNumMeshes())
        return "";

    /* Load mesh */
    auto mesh = model->getMeshes().at(index);
    return mesh->name.c_str();
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

void freeMemory_intArr(int* data)
{
    if (!data) return;
    delete[] data;
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

const char** getAllMeshMorphs(void* pSkinModel, const int meshIndex, int* numShapes)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(meshIndex);

    /* Check if mesh has empty morphs */
    if (mesh->blendshapes.size() == 0)
        return nullptr;

    /* Collect all object shape names */
    std::vector<std::string*> shapeNames;
    for (auto& shape : mesh->blendshapes) {
        shapeNames.push_back(&shape.name); }

    // Convert std::vector<std::string> to array of char pointers
    *numShapes = static_cast<int>(shapeNames.size());
    const char** arr = new const char* [*numShapes];
    for (size_t i = 0; i < *numShapes; ++i) {
        arr[i] = shapeNames[i]->c_str();
    }

    return arr;
}


const float* getMeshBlendShape(void* pSkinModel, const int meshIndex, const int shapeIndex, int* size) 
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(meshIndex);
    if (shapeIndex > mesh->blendshapes.size())
        return nullptr;

    /* Get all blendshape position data */
    auto shape = &mesh->blendshapes.at(shapeIndex);
    const float* verts = shape->vertices.data();

    *size = shape->vertices.size();
    return verts;
}

const int* getVtxMorphIndices(void* pSkinModel, const int meshIndex, const int shapeIndex, int* size)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(meshIndex);
    if (shapeIndex > mesh->blendshapes.size())
        return nullptr;

    /* Get all blendshape position data */
    auto shape = &mesh->blendshapes.at(shapeIndex);
    *size = shape->vtxMorphs.size();
    if (*size == 0)
        return nullptr;

    /* Create an array of all affected vertex indices */
    int* indices = shape->vtxMorphs.data();
    return indices;
}

const char** getAllFaceGroups(void* pSkinModel, const int meshIndex, int* size)
{
    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return nullptr;

    /* Load mesh */
    auto mesh = model->getMeshes().at(meshIndex);

    // Collect all group names
    std::vector<std::string*> groups;
    for (auto& faceGrp : mesh->groups) {
        auto name = &faceGrp.material.name;
        groups.push_back(name);
    }

    // Convert std::vector<std::string> to array of char pointers
    *size = static_cast<int>(groups.size());
    const char** arr = new const char* [*size];
    for (size_t i = 0; i < *size; ++i) {
        arr[i] = groups[i]->c_str();
    }

    return arr;
}

void getMaterialFaceGroup(void* pSkinModel, const int meshIndex, const int groupIndex, int* faceBegin, int* faceSize) 
{
    /* Define default values*/
    *faceBegin = -1;
    *faceSize  = -1;

    // Convert void pointer back to CSkinModel pointer
    CSkinModel* model = static_cast<CSkinModel*>(pSkinModel);
    if (!model || meshIndex > model->getNumMeshes())
        return;

    /* Load mesh */
    auto mesh = model->getMeshes().at(meshIndex);
    if (groupIndex > mesh->groups.size())
        return;

    FaceGroup& group = mesh->groups.at(groupIndex);
    *faceBegin = group.faceBegin;
    *faceSize  = group.numTriangles;
}
