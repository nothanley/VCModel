// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include "dll/interfaceskinmodel.h"
#include "materialgen.h"
#include "wavefront.h"
#include "model_utils.h"

int main()
{
    //ModelUtils::doDebugModelRigCheck();
    //return 0;

    // Scope heap memory test - 
    {
		//CModelContainer mdlFile("C:/Users/brian/Desktop/Blockout_Plane.mcd");
        CModelContainer mdlFile("C:/Users/brian/Desktop/og_basemodel.mcd");
        //CModelContainer mdlFile("C:/Users/brian/Desktop/2k25-0173_Reference_Cube.mcd");
        //CModelContainer mdlFile("C:/Users/brian/Desktop/0173_Reference_Cube.mcd");
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

        CModelSerializer_2_15 serializer(model.get());
        serializer.save("C:/Users/brian/Desktop/output.mcd");

        //for (auto& mesh : model->getMeshes())
            //mesh->calculateTangentsBinormals();

        CModelContainer reloadFile("C:/Users/brian/Desktop/output.mcd");
        reloadFile.load();
    }
     
    printf("\nAll operations complete.");
}

