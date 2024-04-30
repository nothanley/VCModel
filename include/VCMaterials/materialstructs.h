#include <string>
#include <vector>
#include <variant>
#pragma once

#define variant_string std::get<std::string>
#define variant_float std::get<float>
#define variant_list std::get<std::vector<int16_t>>
#define variant_uint std::get<uint32_t>
#define variant_hash std::get<uint64_t>

enum enMaterialFormats
{
	MTLs = 0x734c544d,
	MTL = 0x214c544d,
};

enum enNodeTypes
{
	NUL,
	enFLOAT,
	enLIST,
	enINT,
	enSTRING,
	enHASH
};

enum enTypeHashes
{
	enFLOAT_HASH = 259121563,
	enLIST_HASH = 578531224,
	enINT_HASH = 2090796325,
	enSTRING_HASH = 479440892,
	enLONGLONG_HASH = 2090320585,
};

struct StPropertyNode {
	std::string name;
	int16_t type = -1;
	std::variant<float, std::vector<int16_t>, uint32_t, std::string, uint64_t> value;
};

struct StMaterial {
	std::string name;
	std::string shader;
	std::string type;

	int16_t flag0;
	int8_t  flag1;
	int8_t  flag2;
	int8_t  flag3;
	int16_t flag4;

	std::vector<StPropertyNode> nodes;
};
