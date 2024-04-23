#include "../meshstructs.h"
#pragma once

class CDataBuffer
{
public:
	static 
		void getStringTable(char* stringBuffer, std::vector<std::string>& stringTable);

	static 
		void getModelBones_2_5(char* buffer, const uintptr_t& size, std::vector<RigBone*>& bones, const std::vector<std::string>& stringTable);

	static 
		void getModelBones_2_8(char* buffer, const uintptr_t& size, std::vector<RigBone*>& bones, const std::vector<std::string>& stringTable);

	static 
		void getMaterials(char* buffer, const uintptr_t& size, std::vector<Material>& materials, const std::vector<std::string>& stringTable);

	static 
		void getMeshes(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, const std::vector<std::string>& stringTable);

	static 
		void getLods(char* buffer, const uintptr_t& size, std::vector<Mesh*>& meshTable, const std::vector<Material>& mtlTable, const std::vector<std::string>& strings);
};


