// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"
#include "materialgen.h"
#include "wavefront.h"

int main()
{

    // Scope heap memory test - 
    {
        CModelContainer mdlFile("C:/Users/wauke/Desktop/mask_12500.mcd");
        //CModelContainer mdlFile("C:/Users/wauke/Desktop/0002.yobj");

        mdlFile.load();
        auto model = mdlFile.getModel();

        //if (true)
            //model->linkMaterialsFile("C:/Users/wauke/Desktop/out.mcd");

        //model->injectObj("C:/Users/wauke/Desktop/test.obj", 0);
        //model->linkMaterialsFile("C:/Users/wauke/Desktop/1025_Attire.mtls");

        //CMaterialGen mtlGen(model, "material_presets.json");
        //mtlGen.save(
        //    CMaterialGen::get_mtls_path("C:/Users/wauke/Desktop/123_Attire.mdl").c_str()
        //);

        CModelSerializer_2_9 serializer(model.get());
        serializer.save("C:/Users/wauke/Desktop/output.mcd");

        //for (auto& mesh : model->getMeshes())
            //mesh->calculateTangentsBinormals();

        CModelContainer reloadFile("C:/Users/wauke/Desktop/output.mcd");

        reloadFile.load();
    }

    printf("");
}

