// VCModel.cpp : This file contains the 'main' function. Program execution begins and ends there. //
#include <VCModel>
#include <exception>
#include <vector>
#include "dll/interfaceskinmodel.h"
#include "materialgen.h"
#include "wavefront.h"
#include "model_utils.h"
#include "lodbias_debug.h"

int main()
{
    try
    {
    //ModelUtils::doDebugModelRigCheck();
    //return 0;

    // Scope heap memory test - 
    {
        std::vector<std::shared_ptr<CSkinModel>> keepAlive;

        const char* paths[] = {
            //"C:/Users/brian/Desktop/Blockout_Plane.mcd",
            //"C:/Users/brian/Desktop/og_basemodel.mcd"
            //"C:/Users/brian/Desktop/basemodel.mcd"
			"C://Users/brian/Desktop/1034_Attire.mcd"
        };

        for (const char* path : paths)
        {
            printf("Loading %s...\n", path);
            fflush(stdout);
            CModelContainer mdlFile(path);
            mdlFile.load();
            auto model = mdlFile.getModel();
            if (model) keepAlive.push_back(model);
            printf("Loaded %s.\n", path);
            fflush(stdout);

            if (model)
            {
                printf("Serializing %s...\n", path);
                fflush(stdout);
                CModelSerializer_2_15 serializer(model.get());
                serializer.save("C:/Users/brian/Desktop/output.mcd");

                CModelContainer reloadFile("C:/Users/brian/Desktop/output.mcd");
                reloadFile.load();
                printf("Reloaded output.mcd.\n");
                fflush(stdout);
            }
        }
    }

         
        printf("\nAll operations complete.");
    }
    catch (const std::exception& e)
    {
        printf("Unhandled exception: %s\n", e.what());
        return 1;
    }
    catch (...)
    {
        printf("Unhandled unknown exception.\n");
        return 1;
    }
}

