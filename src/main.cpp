// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"
#include "materialgen.h"

int main()
{
    // Scope heap memory test - 
    {
        CModelContainer mdlFile("C:/Users/wauke/Desktop/123_Attire.mdl");
        CSkinModel* model = mdlFile.getModel();


        //CModelSerializer serializer(model);
        //serializer.save("C:/Users/wauke/Desktop/OUT_VCMODEL.mdl");

        //for (auto& mesh : model->getMeshes())
            //mesh->calculateTangentsBinormals();

        CMaterialGen mtlGen(model, "peresets.json");
        mtlGen.save( 
            CMaterialGen::get_mtls_path("C:/Users/wauke/Desktop/123_Attire.mdl").c_str()
        );

        if (model)
            delete model;
    }

}

