#include <skinmodelpoly.h>
#pragma once 

struct YukesPOF0
{
    char*    ymxen       = NULL;
    char*    pof0        = NULL;
    char*    meshTable   = NULL;
    char*    armature    = NULL;
    char*    stringTbl   = NULL;
    char*    texTable    = NULL;
    uint32_t numMeshes   = NULL;
    uint32_t numBones    = NULL;
    uint32_t numTexs     = NULL;
    uint32_t numTexs     = NULL;
    uint32_t numStrings  = NULL;
};

class CYukesSkinModel : public CSkinModel
{
public:
    CYukesSkinModel(char* data, CModelContainer* pParent);

private:
    void loadData() override;

private:
    void readHeader();
    void readMesh();
    void readArmature();
    void readBone();

private:
    void loadVerts();
    void loadNorms();
    void loadTris();
    void loadWeights(Mesh& mesh, char* table, const int segments);
    void loadTexCoords();

private:
    // todo: move these to yukes header struct ...
    YukesPOF0 m_info;
};

