#include <string>
#include <vector>

enum {
	POSITION = 1656822844,
	NORMALS = 2865655510,
	BINORMALS = 2552595965,
	TANGENTS = 2555205862,
	BLENDWEIGHTS = 3362741239,
	BLENDINDICES = 2622613563,
	TEXCOORDS = 680036153,
	COLOR = 229197112,

	R16_G16_B16_A16 = 3631935628,
	R32_G32_B32 = 251819603,
	R8_G8_B8_A8 = 3503152076,
	R32_G32 = 1382734287,
	R32 = 0x0b867096,
	R8 = 5859695,
};

namespace MeshSerializer {

	class Data {

	public:

		static char* roundPointerToNearest4(char* ptr);

		static constexpr unsigned int hash(const char* s, int off = 0);

		static int roundUp(int numToRound, int multiple);

		static float bitFloat(float value, std::string signValue);

		static float shortFloat(float value, std::string signValue);

		static void getDataSet(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet);

		static int getDataSetSize(int verts, std::string blockType);

		static void getData_R32_G32_B32(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet);

		static void getData_R8_G8_B8_A8(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet);

		static void getData_R16_G16_B16_A16(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet);

		static void getData_R32_G32(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet);

		static void getData_R8(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet);


	};

}
