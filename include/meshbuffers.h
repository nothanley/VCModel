#include <vector>
#include "windows.h"
#include <map>
#include "BinaryIO.h"
#include <iostream>
using namespace BinaryIO;

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

namespace MeshBuffers {

	class Data {

	public:

		static char* roundPointerToNearest4(char* ptr)
		{
			std::ptrdiff_t diff = ptr - reinterpret_cast<char*>(0); // Get the pointer difference from nullptr
			std::ptrdiff_t roundedDiff = (diff + 3) & ~3; // Round up to the nearest multiple of 4
			char* roundedPtr = reinterpret_cast<char*>(roundedDiff); // Convert the rounded difference back to a pointer
			return roundedPtr;
		}

		static constexpr unsigned int hash(const char* s, int off = 0) {
			return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
		}

		static int roundUp(int numToRound, int multiple) {
			int remainder;

			if (multiple == 0)
				return numToRound;
			remainder = numToRound % multiple;
			if (remainder == 0)
				return numToRound;

			return (numToRound + multiple - remainder);
		}

		static float bitFloat(float value, std::string signValue) {

			if (signValue == "snorm")
				value = float(value / 127.0);
			else if (signValue == "unorm")
				value = float(value / 255.0);
			return value;
		}

		static float shortFloat(float value, std::string signValue) {
			if (signValue == "snorm")
				value = float(value / 32767.0);
			else if (signValue == "unorm")
				value = float(value / 65535.0);
			return value;
		}

		static void getDataSet(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet) {
			unsigned int buf = hash(blockType.c_str());

			switch (buf)
			{
				case R16_G16_B16_A16:
					getData_R16_G16_B16_A16(data, verts, dataType, blockType, dataSet);
					break;
				case R32_G32_B32:
					getData_R32_G32_B32(data, verts, dataType, blockType, dataSet);
					break;
				case R8_G8_B8_A8:
					 getData_R8_G8_B8_A8(data, verts, dataType, blockType, dataSet);
					break;
				case R32_G32:
					getData_R32_G32(data, verts, dataType, blockType, dataSet);
					break;
				case R8:
					getData_R8(data, verts, dataType, blockType, dataSet);
					break;
				default:
					break;
			}

		}

		static int getDataSetSize(int verts, std::string blockType) {

			unsigned int size = verts;
			unsigned int buf = hash(blockType.c_str());
			switch (buf)
			{
				case R16_G16_B16_A16:
					size *= 8;
					break;
				case R32_G32_B32:
					size *= 12;
					break;
				case R8_G8_B8_A8:
					size *= 4;
					break;
				case R32:
					size *= 4;
					break;
				case R32_G32:
					size *= 8;
					break;
				case R8:
					size *= 1;
					break;
				default:
					break;
			}

			//check alignment
			size = roundUp(size, 4);

			return size;
		}

		static void getData_R32_G32_B32(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet) {

			for (int i = 0; i < verts; i++) {
				float r32 = ReadFloat(data);
				float g32 = ReadFloat(data);
				float b32 = ReadFloat(data);

				dataSet.push_back(r32);
				dataSet.push_back(g32);
				dataSet.push_back(b32);
			}
		}

		static void getData_R8_G8_B8_A8(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet) {

			for (int i = 0; i < verts; i++) {
				float r8f, g8f, b8f, a8f;

				if (dataType == "snorm") {
					int8_t r8 = ReadInt8(data);
					int8_t g8 = ReadInt8(data);
					int8_t b8 = ReadInt8(data);
					int8_t a8 = ReadInt8(data);

					r8f = bitFloat(r8, dataType);
					g8f = bitFloat(g8, dataType);
					b8f = bitFloat(b8, dataType);
					a8f = bitFloat(a8, dataType);
				}
				else if (dataType == "unorm") {
					uint8_t r8 = ReadUInt8(data);
					uint8_t g8 = ReadUInt8(data);
					uint8_t b8 = ReadUInt8(data);
					uint8_t a8 = ReadUInt8(data);

					r8f = bitFloat(r8, dataType);
					g8f = bitFloat(g8, dataType);
					b8f = bitFloat(b8, dataType);
					a8f = bitFloat(a8, dataType);
				}
				else if (dataType == "sint") {
					int8_t r8 = ReadInt8(data);
					int8_t g8 = ReadInt8(data);
					int8_t b8 = ReadInt8(data);
					int8_t a8 = ReadInt8(data);

					r8f = r8;
					g8f = g8;
					b8f = b8;
					a8f = a8;
				}
				else if (dataType == "uint") {
					uint8_t r8 = ReadUInt8(data);
					uint8_t g8 = ReadUInt8(data);
					uint8_t b8 = ReadUInt8(data);
					uint8_t a8 = ReadUInt8(data);

					r8f = r8;
					g8f = g8;
					b8f = b8;
					a8f = a8;
				}


				dataSet.push_back(r8f);
				dataSet.push_back(g8f);
				dataSet.push_back(b8f);
				dataSet.push_back(a8f);
			}
		}

		static void getData_R16_G16_B16_A16(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet) {

			for (int i = 0; i < verts; i++) {
				float r8f, g8f, b8f, a8f;

				if (dataType == "snorm") {
					int16_t r8 = ReadInt16(data);
					int16_t g8 = ReadInt16(data);
					int16_t b8 = ReadInt16(data);
					int16_t a8 = ReadInt16(data);

					r8f = shortFloat(r8, dataType);
					g8f = shortFloat(g8, dataType);
					b8f = shortFloat(b8, dataType);
					a8f = shortFloat(a8, dataType);
				}
				else if (dataType == "unorm") {
					uint16_t r8 = ReadUInt16(data);
					uint16_t g8 = ReadUInt16(data);
					uint16_t b8 = ReadUInt16(data);
					uint16_t a8 = ReadUInt16(data);

					r8f = shortFloat(r8, dataType);
					g8f = shortFloat(g8, dataType);
					b8f = shortFloat(b8, dataType);
					a8f = shortFloat(a8, dataType);
				}
				else if (dataType == "sint") {
					int16_t r8 = ReadInt16(data);
					int16_t g8 = ReadInt16(data);
					int16_t b8 = ReadInt16(data);
					int16_t a8 = ReadInt16(data);

					r8f = r8;
					g8f = g8;
					b8f = b8;
					a8f = a8;
				}
				else if (dataType == "uint") {
					uint16_t r8 = ReadUInt16(data);
					uint16_t g8 = ReadUInt16(data);
					uint16_t b8 = ReadUInt16(data);
					uint16_t a8 = ReadUInt16(data);

					r8f = r8;
					g8f = g8;
					b8f = b8;
					a8f = a8;
				}


				dataSet.push_back(r8f);
				dataSet.push_back(g8f);
				dataSet.push_back(b8f);
				dataSet.push_back(a8f);
			}
		}

		static void getData_R32_G32(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet) {

			for (int i = 0; i < verts; i++) {
				float r32 = ReadFloat(data);
				float g32 = ReadFloat(data);

				dataSet.push_back(r32);
				dataSet.push_back(g32);
			}
		}

		static void getData_R8(char*& data, int verts, std::string dataType, std::string blockType, std::vector<float>& dataSet) {

			for (int i = 0; i < verts; i++) {
				float r8f;

				if (dataType == "snorm") {
					int8_t r8 = ReadInt8(data);

					r8f = bitFloat(r8, dataType);
				}
				else if (dataType == "unorm") {
					uint8_t r8 = ReadUInt8(data);

					r8f = bitFloat(r8, dataType);
				}
				else if (dataType == "sint") {
					int8_t r8 = ReadInt8(data);

					r8f = r8;
				}
				else if (dataType == "uint") {
					uint8_t r8 = ReadUInt8(data);

					r8f = r8;
				}

				dataSet.push_back(r8f);
			}
		}


	};

}
