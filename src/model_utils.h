#include <string>
#include <vector>
#include "glm/mat4x4.hpp"
#pragma once

class CSkinModel;
struct RigBone;

struct BonePreDef
{
    std::string name;
    std::string parent;
    glm::mat4 matrix_local;

    RigBone* toRigBone() const;
};

namespace ModelUtils
{
	void rebuildArmatureFromLatest(CSkinModel* model);
	void doDebugModelRigCheck();
}

