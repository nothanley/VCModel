// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>

int main()
{
    CModelContainer mdlFile("C:/Users/wauke/Desktop/baseModel.mdl");
    CSkinModel* model = mdlFile.getModel();

    if (model) {
        delete model;
    }
}
