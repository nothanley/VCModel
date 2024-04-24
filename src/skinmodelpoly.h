#include "skinmodel.h"
#pragma once

class CSkinModel_2_8 : public CSkinModel
{
public:
    CSkinModel_2_8(char* data, CModelContainer* pParent) : CSkinModel(data, pParent) 
    {
        this->loadBuffer();
    }

private:
    void loadBuffer() override;
    void loadModelBones(const uintptr_t& size) override;
    void readBone() override;
    void getMeshMapInfo(Mesh& mesh) override;
};


class CSkinModel_2_5 : public CSkinModel
{
public:
    CSkinModel_2_5(char* data, CModelContainer* pParent) : CSkinModel(data, pParent)
    {
        this->loadBuffer();
    }

private:
    void loadBuffer() override;
    void loadModelBones(const uintptr_t& size) override;
    void readBone() override;
    void getMeshMapInfo(Mesh& mesh) override;
};
