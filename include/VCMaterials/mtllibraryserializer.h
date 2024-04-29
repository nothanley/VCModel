#include "materialserializer.h"
#include "materiallibrary.h"
#include <memory>
#pragma once

class CMtlLibrarySerializer : public CMaterialSerializer
{
public:
	CMtlLibrarySerializer(std::shared_ptr<CMaterialLibrary> library);
	void save(const char* path);

private:
	void createMtlsHeader(char*& data);
	void createMaterialTable(char*& data);
	void serialize(char*& dst, uint32_t& size);

private:
	std::shared_ptr<CMaterialLibrary> m_materialLibrary;
};

