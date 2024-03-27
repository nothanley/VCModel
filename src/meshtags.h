#pragma once


enum enModelFormats
{
	MDL_MAGIC = 0x214c444d,
	MDL_VERSION_2_5 = 0x25,
	MDL_VERSION_2_8 = 0x28,
};

enum enMeshBfTags
{
	MDL  = 0x4D444C21,
	TEXT = 0x54584554,
	BONE = 0x454e4f42,
	AtPt = 0x74507441,
	MTL  = 0x214c544d,
	MBfD = 0x4466424d,
	LODs = 0x73444f4c,
	ENDP = 0x454E4421,
	ENDM = 0x454E444D,
};