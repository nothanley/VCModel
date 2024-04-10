// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>

int main()
{
    CModelContainer mdlFile("C:/Users/wauke/Desktop/baseModel.mdl");
    CSkinModel* model = mdlFile.getModel();

    auto mesh = model->getMeshes().front();
    mesh->convertSplitNorms();

    for (auto& mesh : model->getMeshes()) {
        auto table = model->getStringTable();
        auto skindata = mesh->skin.unpack(*table);

        delete skindata;
    }

    if (model)
        delete model;
}