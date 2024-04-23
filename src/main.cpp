// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"

int main()
{
    // Scope heap memory test - 
    {
        CModelContainer mdlFile("C:/Users/wauke/Desktop/basemodel.mdl");
        CSkinModel* model = mdlFile.getModel();


        CModelSerializer serializer(model);
        serializer.save("C:/Users/wauke/Desktop/OUT_VCMODEL.mdl");

        if (model)
            delete model;
    }

}

