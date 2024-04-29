#include <sstream>
#include <memory>
#include "materialstructs.h"
#pragma once

struct StMtlDataBf
{
	std::stringstream stream;
	std::string ds() { return stream.str(); }
	uint32_t size()  { return stream.str().size(); }
};

class CMaterialSerializer
{
protected:
	void serializeMaterial(const StMaterial& material);
	std::vector< std::shared_ptr<StMtlDataBf> > m_materialBfs;

private:
	uint32_t getStringAddress(const std::string& target);
	void writeMaterialHeader(std::stringstream& stream);
	void serializeMaterialNodes(StMtlDataBf& data, const StMaterial& material);
	void serializeInfoBuffer(std::stringstream& stream, const StMaterial& material);

private:
	std::stringstream m_stringtable;
};

