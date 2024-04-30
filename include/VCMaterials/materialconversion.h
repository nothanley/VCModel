#include <memory>
#pragma once

class CMaterialLibrary;
class CMaterialTemplates;

class CMaterialConv
{

public:
	static void convert_legacy_to_2024(CMaterialTemplates& presets, std::shared_ptr<CMaterialLibrary>& library);

};

