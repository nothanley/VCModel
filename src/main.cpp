// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"
#include "materialgen.h"
#include "wavefront.h"

int main()
{

    // Scope heap memory test - 
    {
        //CModelContainer mdlFile("C:/Users/wauke/Desktop/basemodel.mdl");
        CModelContainer mdlFile("C:/Users/wauke/Desktop/file.yobj");

        mdlFile.load();
        auto model = mdlFile.getModel();

        //model->injectObj("C:/Users/wauke/Desktop/test.obj", 0);
        //model->linkMaterialsFile("C:/Users/wauke/Desktop/123_Attire.mdl");

        //CMaterialGen mtlGen(model, "material_presets.json");
        //mtlGen.save(
        //    CMaterialGen::get_mtls_path("C:/Users/wauke/Desktop/123_Attire.mdl").c_str()
        //);

        //CModelSerializer serializer(model.get());
        //serializer.save("C:/Users/wauke/Desktop/save.mdl");

        //for (auto& mesh : model->getMeshes())
            //mesh->calculateTangentsBinormals();

        printf("");
    }

    printf("");
}

