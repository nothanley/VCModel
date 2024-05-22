#include "blendshapes.h"
#pragma once

class vCMeshShapes_2020 : public vCMeshShapes
{

public:
	vCMeshShapes_2020(char*& data, const std::vector<std::string>& stringTable, Mesh* mesh);
	void load() override;

protected:
	void getMorphs();
	void loadBlendshapeData(StBlendShape& shape);
};