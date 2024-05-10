// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"
#include "materialgen.h"

int main()
{
    // Scope heap memory test - 
    {
        CModelContainer mdlFile("C:/Users/wauke/Desktop/stress_1m.mdl");
        CSkinModel* model = mdlFile.getModel();

        //model->linkMaterialsFile("C:/Users/wauke/Desktop/123_Attire.mdl");

        //CMaterialGen mtlGen(model, "material_presets.json");
        //mtlGen.save(
        //    CMaterialGen::get_mtls_path("C:/Users/wauke/Desktop/123_Attire.mdl").c_str()
        //);

        CModelSerializer serializer(model);
        serializer.save("C:/Users/wauke/Desktop/OUT_VCMODEL.mdl");

        //for (auto& mesh : model->getMeshes())
            //mesh->calculateTangentsBinormals();

        if (model)
            delete model;
    }

}

