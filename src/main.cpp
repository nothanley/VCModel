// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"

int main()
{
    CModelContainer mdlFile("C:/Users/wauke/Desktop/baseModel_Cena_2023.mdl");
    CSkinModel* model = mdlFile.getModel();

    /* Format mesh data - todo: make this default */
    for (auto& mesh : model->getMeshes()) {
        mesh->convertSplitNorms();
    }

    CModelSerializer serializer(model);
    serializer.save("C:/Users/wauke/Desktop/OUT_VCMODEL.mdl");

    if (model)
        delete model;
}