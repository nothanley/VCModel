#include <materialbuffer.h>
#pragma once

class CSkinModel;
class CMaterialFile;

class CMaterialLibrary : CMaterialBuffer
{
public:
    CMaterialLibrary() : CMaterialBuffer(nullptr, nullptr){}
	CMaterialLibrary(char* data, CMaterialFile* pParent);
    ~CMaterialLibrary(){}

public:
	void fromModel(CSkinModel* model, const char* presets_json);
	void loadlibrary();
	void updateLinks(const char* material_path);

public:
    StMaterial getMaterial(const int index);
	StMaterial* at(const int index);
	int indexOf(const char* target_name);
    void addMaterial(StMaterial& material, const char* link_path="");
	const std::vector<StMaterial>& getMaterials() const { return m_materials; }
    const int numMaterials() const { return  m_materials.size(); }
	void retarget(const StMaterial& material);
	void removeDuplicates();

public:
	static int indexOf(const std::vector<StMaterial>& materials, const StMaterial& material);

private:
	void loadMaterialTable();
	void loadMaterials();
};

