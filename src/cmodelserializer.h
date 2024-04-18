#include <string>
#include <vector>
#pragma once

struct StModelBf {
	std::string type;
	uint32_t size;
	char* data = nullptr;
};

const std::vector<std::string> STREAM_TABLE {
	"POSITION", "R32_G32_B32","float", "NORMAL", "R8_G8_B8_A8", "snorm",
	"TANGENT","BINORMAL","R8", "COLOR","unorm","TEXCOORD",
	"R32_G32","R16_G16_B16_A16","BLENDINDICES", "uint","BLENDWEIGHTS","R32_G32_B32_A32",""
};

class CSkinModel;
class CModelSerializer
{
public:
	CModelSerializer(CSkinModel* target);
	void save(const char* path);

private:
	void serialize();
	void buildStringTable();

private:
	void createTextBuffer();
	void createBoneBuffer();


protected:
	std::vector<StModelBf>   m_dataBuffers;
	std::vector<std::string> m_stringTable;
	std::string m_savePath;
	CSkinModel* m_model;
};

