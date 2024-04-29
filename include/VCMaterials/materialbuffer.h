#include <materialstructs.h>
#pragma once

class CMaterialFile;
class CMaterialBuffer
{
public:
	CMaterialBuffer(char* data, CMaterialFile* parent);

protected:
	void loadMaterialBuffer(StMaterial& material);

private:
	void validateBuffer();
	void loadStringTable();
	std::string getString();

private:
	void loadMaterialData(StMaterial& material);
	void getMaterialInfo(StMaterial& material);
	void readPropertyNode(StMaterial& material);

private:
	char* m_pStringTable;

protected:
	char* m_data;
	CMaterialFile* m_parent;
	std::vector<StMaterial> m_materials;
};
