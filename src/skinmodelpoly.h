#include "skinmodel.h"
#pragma once

class CSkinModel_2_8 : public CSkinModel
{
public:
    CSkinModel_2_8(char* data, CModelContainer* pParent) : CSkinModel(data, pParent) 
    {
        this->loadData();
        this->loadBuffer();
    }

private:
    void loadModelBones(const uintptr_t& size) override;
    void readBone() override;
    void getMeshMapInfo(Mesh& mesh) override;
};


class CSkinModel_2_5 : public CSkinModel
{
public:
    CSkinModel_2_5(char* data, CModelContainer* pParent) : CSkinModel(data, pParent)
    {
        this->loadData();
        this->loadBuffer();
    }

private:
    void loadModelBones(const uintptr_t& size) override;
    void readBone() override;
    void getMeshMapInfo(Mesh& mesh) override;
};

class CSkinModel_2_0 : public CSkinModel
{
public:
    CSkinModel_2_0(char* data, CModelContainer* pParent) : CSkinModel(data, pParent)
    {
        this->loadData();
        this->loadBuffer();
    }

private:
    void buildMesh(Mesh& mesh) override;
    void loadData() override;
    void loadModelBones(const uintptr_t& size) override;
    void readBone() override;
    void getMeshMapInfo(Mesh& mesh) override;
};

class CSkinModel_1_1 : public CSkinModel
{
public:
    CSkinModel_1_1(char* data, CModelContainer* pParent) : CSkinModel(data, pParent)
    {
        this->loadData();
        this->loadBuffer();
    }

private:
    virtual void getMorphWeights(Mesh& mesh) override;
    void buildMesh(Mesh& mesh) override;
    void loadData() override;
    void loadModelBones(const uintptr_t& size) override;
    void readBone() override;
    void getMeshMapInfo(Mesh& mesh) override;
};
