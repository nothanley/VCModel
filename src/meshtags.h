#pragma once


enum enModelFormats
{
	MDL_MAGIC = 0x214c444d,
	MDL_VERSION_2_8 = 0x28, // WWE 2K24 - Visual Concepts MDL v2.5
	MDL_VERSION_2_5 = 0x25, // WWE 2K23 - Visual Concepts MDL v2.8
	MDL_VERSION_2_0 = 0x20, // WWE 2K22 - Visual Concepts MDL v2.0
	MDL_VERSION_1_1 = 0x1D, // WWE 2K20 - Visual Concepts MDL v1.13
};

enum enMeshBfTags
{
	MDL  = 0x4D444C21,
	TEXT = 0x54584554,
	BONE = 0x454e4f42,
	AtPt = 0x74507441,
	GtPt = 0x74507447,
	MTL_  = 0x214c544d,
	MBfD = 0x4466424d,
	LODs = 0x73444f4c,
	ENDM = 0x454E444D,
	END  = 0x21444e45,
};