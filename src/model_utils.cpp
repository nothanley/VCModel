#include "model_utils.h"
#include <VCModel>
#include <algorithm>
#include "global_defs.hpp"

inline static bool contains(std::string str, std::string substr)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::transform(substr.begin(), substr.end(), substr.begin(), ::tolower);
    return str.find(substr) != std::string::npos;
}

inline static bool startsWith(std::string str, std::string prefix)
{
    if (str.length() < prefix.length()) return false;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
    return str.substr(0, prefix.length()) == prefix;
}

RigBone* getBone(std::vector<RigBone*> bones, std::string& target)
{
    for (auto& bone : bones)
    {
        if (bone->name == target)
            return bone;
    }

    return nullptr;
}

RigBone* BonePreDef::toRigBone() const
{
    RigBone* bone      = new RigBone();
    bone->name         = this->name;
    bone->matrix_local = this->matrix_local;
    return bone;
}

void ModelUtils::rebuildArmatureFromLatest(CSkinModel* model)
{
    auto bones = model->getBones();
    int count = 0;

    for (auto& def : gBoneDefs)
    {
        auto bone = model->find_bone(def.name.c_str());
        if (bone)
        {
            continue;
        }

        auto parent = model->find_bone(def.parent.c_str());
        if (parent)
        {
            RigBone* new_bone = def.toRigBone();
            parent->children.push_back(new_bone);
            new_bone->set_parent(parent);
            model->push_bone(new_bone);
            count++;
        }
    }

    if (count > 0) 
    {
        //model->sort_bones(); todo: fix indices ...
        std::cout << "[ModelService] Added " << count << " missing bone(s) to model." << std::endl;
    }

    return;
};


void ModelUtils::doDebugModelRigCheck()
{
    //CModelContainer file("C:/Users/wauke/Desktop/basemodel_tiff_22.mdl");
    //CModelContainer file("C:/Users/wauke/Desktop/basemodel_2k23.mdl");
    //file.load();
    //auto model = file.getModel();
    //if (!model) return;

    //ModelUtils::rebuildArmatureFromLatest(model.get());

    //CModelSerializer_2_9 out(model.get());
    //out.save("C:/Users/wauke/Desktop/output.mcd");
    //out.save("C:/Users/wauke/Desktop/bake_environment/bake-me/Characters/1025_Cody_Rhodes/BaseModel/basemodel.mcd");
}
