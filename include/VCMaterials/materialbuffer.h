#include <materialstructs.h>
#pragma once

class CMaterialFile;

class CMaterialBuffer
{
public:
	CMaterialBuffer(char* data, CMaterialFile* parent);

protected:
	std::string getString();
	virtual void readPropertyNode(StMaterial& material);

protected:
	virtual void loadMaterialBuffer(StMaterial& material);
	virtual void loadMaterialData(StMaterial& material);
	virtual void getMaterialInfo(StMaterial& material);
	static inline std::vector<int16_t> getListParams(char*& stream);

private:
	void validateBuffer();
	void loadStringTable();

protected:
	char* m_pStringTable;
	char* m_data;
	CMaterialFile* m_parent;
	std::vector<StMaterial> m_materials;
};

