// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"

int main()
{
    CModelContainer mdlFile("C:/Users/wauke/Desktop/baseModel_Cena_2023.mdl");
    CSkinModel* model = mdlFile.getModel();

    auto mesh = model->getMeshes().front();
    mesh->convertSplitNorms();

    for (auto& mesh : model->getMeshes()) {
        auto table    = model->getStringTable();
        auto skindata = mesh->skin.unpack(*table);

        if (mesh->colors.size() > 0) {
            int channels = mesh->colors.front().map.size() / mesh->numVerts;
            printf("");
        }

        delete skindata;
    }

    if (model)
        delete model;
}