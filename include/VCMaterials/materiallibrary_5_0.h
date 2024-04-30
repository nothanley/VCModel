#include "materiallibrary.h"
#pragma once

class CMaterialLibrary_5_0 : public CMaterialLibrary
{

public:
	CMaterialLibrary_5_0() : CMaterialLibrary(nullptr, nullptr) {}
	CMaterialLibrary_5_0(char* data, CMaterialFile* pParent);
	~CMaterialLibrary_5_0();

public:
	void loadlibrary() override;

private:
	void loadMaterialTable() override;
	void loadMaterials() override;
	void loadMaterialData(StMaterial& material) override;
	void readPropertyNode(StMaterial& material) override;
};

