#include "BinaryIO.h"
#include <fstream>
#include <windows.h> 
#include <winsock.h>
#include <istream>
#include <vector>
#include <sstream>
using namespace std;

uint8_t 
BinaryIO::extractValue(uint32_t value, uint8_t place) {
	return (value >> (place * 8)) & 0xFF;
}
uint8_t 
BinaryIO::extractValue_64(uint64_t value, uint8_t place) {
	return (value >> (place * 8)) & 0xFF;
}

uint64_t 
BinaryIO::swapLongEndian(const uint64_t& X) {
	return _byteswap_uint64(X);
}

std::string 
BinaryIO::ReadString(char*& data) {
	std::string str(data);
	data += str.size() + sizeof(char);
	return str;
}

std::string
BinaryIO::ReadString(char*& data, int size) {
	std::string value = reinterpret_cast<const char*>(data);
	data += sizeof(size);
	return value;
}

int8_t
BinaryIO::ReadInt8(char*& buffer) {
	int8_t value = *reinterpret_cast<const int8_t*>(buffer);
	buffer += sizeof(int8_t);
	return value;
}

uint8_t 
BinaryIO::ReadUInt8(char*& buffer) {
	uint8_t value = *reinterpret_cast<const uint8_t*>(buffer);
	buffer += sizeof(uint8_t);
	return value;
}

uint16_t
BinaryIO::ReadUInt16(char*& buffer, bool swap) {
	uint16_t value = *reinterpret_cast<const uint16_t*>(buffer);
	buffer += sizeof(uint16_t);
	return swap ? ntohs(value) : value;
}

uint32_t
BinaryIO::ReadUInt32(char*& buffer, bool swap) {
	uint32_t value;

	value = *reinterpret_cast<const uint32_t*>(buffer);
	buffer += sizeof(uint32_t);
	return swap ? ntohl(value) : value;
}

int16_t BinaryIO::ReadInt16(char*& buffer, bool swap)
{
	int16_t value = *reinterpret_cast<const int16_t*>(buffer);
	buffer += sizeof(int16_t);
	return swap ? ntohs(value) : value;
}

int32_t BinaryIO::ReadInt32(char*& buffer, bool swap)
{
	int32_t value = *reinterpret_cast<const int32_t*>(buffer);
	buffer += sizeof(int32_t);
	return swap ? ntohl(value) : value;
}

int64_t BinaryIO::ReadInt64(char*& buffer, bool swap)
{
	int64_t value = *reinterpret_cast<const int64_t*>(buffer);
	buffer += sizeof(int64_t);
	return swap ? ntohl(value) : value;
}

float
BinaryIO::ReadFloat(char*& buffer) {
	float value = *reinterpret_cast<const float*>(buffer);
	buffer += sizeof(uint32_t);
	return value;
}

uint64_t
BinaryIO::ReadUInt64(char*& buffer, bool swap) {
	uint64_t value = *reinterpret_cast<const uint64_t*>(buffer);
	buffer += sizeof(uint64_t);
	return swap ? swapLongEndian(value) : value;
}

void 
BinaryIO::WriteUInt8(char*& buffer, const uint8_t& value) {
	*reinterpret_cast<uint8_t*>(buffer) = value;
	buffer += sizeof(uint8_t);
}

void 
BinaryIO::WriteUInt16(char*& buffer, const uint16_t& value) {
	*reinterpret_cast<uint16_t*>(buffer) = value;
	buffer += sizeof(uint16_t);
}

void 
BinaryIO::WriteUInt32(char*& buffer, const uint32_t& value) {
	*reinterpret_cast<uint32_t*>(buffer) = value;
	buffer += sizeof(uint32_t);
}

void 
BinaryIO::WriteUInt64(char*& buffer, const uint64_t& value) {
	*reinterpret_cast<uint64_t*>(buffer) = value;
	buffer += sizeof(uint64_t);
}

void
BinaryIO::WriteSInt8(char*& buffer, const int8_t& value) {
	*reinterpret_cast<int8_t*>(buffer) = value;
	buffer += sizeof(int8_t);
}

void
BinaryIO::WriteSInt16(char*& buffer, const int16_t& value) {
	*reinterpret_cast<int16_t*>(buffer) = value;
	buffer += sizeof(int16_t);
}

void
BinaryIO::WriteSInt32(char*& buffer, const int32_t& value) {
	*reinterpret_cast<int32_t*>(buffer) = value;
	buffer += sizeof(int32_t);
}

void
BinaryIO::WriteSInt64(char*& buffer, const int64_t& value) {
	*reinterpret_cast<int64_t*>(buffer) = value;
	buffer += sizeof(int64_t);
}


void 
BinaryIO::WriteFloat(char*& buffer, const float& value) {
	*reinterpret_cast<float*>(buffer) = value;
	buffer += sizeof(float);
}

void 
BinaryIO::WriteData(char*& buffer, char* data, size_t size) {
	memcpy(buffer, data, size);
	buffer += size;
}

void 
BinaryIO::WriteString(char*& buffer, const std::string& string)
{
	int characters = string.size()+1;
	memcpy(buffer, string.c_str(), characters);
	buffer += characters;
}

void
BinaryIO::ReadData(std::istream& fileStream, size_t size, std::stringstream& stream) {
	std::vector<char> buffer;
	buffer.resize(size);
	fileStream.read(buffer.data(), size);

	stream.write(buffer.data(), size);
	return;
}

void
BinaryIO::ReadData(std::istream& fileStream, size_t size, std::vector<char>& buffer) {
	buffer.resize(size);
	fileStream.read(buffer.data(), size);
	return;
}

std::string
BinaryIO::ReadString(istream& fs, int size) {
	std::string value;
	value.resize(size);
	fs.read(&value[0], size);

	// Remove any trailing null characters
	size_t nullPos = value.find('\0');
	if (nullPos != std::string::npos)
		value.resize(nullPos);

	return value;
}

int 
BinaryIO::ReadInt(istream& fs) {

	int value = 0;
	fs.read((char*)&value, 4);

	return value;
}

unsigned int 
BinaryIO::ReadUInt(istream& in) {

	unsigned int value = 0;
	in.read((char*)&value, 4);

	return value;
}

uint32_t 
BinaryIO::ReadUInt32(istream& in) {

	uint32_t value = 0;
	in.read((char*)&value, 4);

	return value;
}

int8_t 
BinaryIO::ReadSInt8(istream& in) {

	int8_t value = 0;
	in.read((char*)&value, 1);

	return value;
}

int16_t 
BinaryIO::ReadSInt16(istream& in) {

	int16_t value = 0;
	in.read((char*)&value, 2);

	return value;
}

int32_t 
BinaryIO::ReadSInt32(istream& in) {

	int32_t value = 0;
	in.read((char*)&value, 4);

	return value;
}

uint64_t 
BinaryIO::ReadUInt64(istream& in) {
	uint64_t value = 0;
	in.read((char*)&value, sizeof(uint64_t));
	return value;
}

unsigned short 
BinaryIO::ReadUShort(istream& in) {

	unsigned short value = 0;
	in.read((char*)&value, 2);

	return value;
}

short 
BinaryIO::ReadShort(istream& in) {

	short value = 0;
	in.read((char*)&value, 2);

	return value;
}

uint8_t 
BinaryIO::ReadByte(istream& in) {

	uint8_t value = 0;
	in.read((char*)&value, 1);

	return value;
}

float 
BinaryIO::ReadFloat(istream& in) {

	float value = 0;
	in.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));

	return value;
}

bool 
BinaryIO::ReadBool(istream& in) {
	return (ReadByte(in) != 0);
}

void 
BinaryIO::WriteUInt64(ofstream* fs, uint64_t value) {
	fs->write(reinterpret_cast<char*>(&value), sizeof(uint64_t));
}
void 
BinaryIO::WriteUInt32(ofstream* fs, uint32_t value) {
	fs->write(reinterpret_cast<char*>(&value), sizeof(uint32_t));
}
void
BinaryIO::WriteUInt16(ofstream* fs, uint16_t value) {
	fs->write(reinterpret_cast<char*>(&value), sizeof(uint16_t));
}
void
BinaryIO::WriteByte(ofstream* fs, uint8_t value) {
	fs->write(reinterpret_cast<char*>(&value), sizeof(uint8_t));
}
void
BinaryIO::WriteBool(ofstream* fs, bool flag) {
	fs->write(reinterpret_cast<char*>(&flag), sizeof(bool));
}
void 
BinaryIO::WriteInt32(ofstream* fs, int32_t value) {
	fs->write(reinterpret_cast<char*>(&value), sizeof(int32_t));
}
void
BinaryIO::WriteFloat(ofstream* fs, float value) {
	fs->write(reinterpret_cast<char*>(&value), sizeof(float));
}
void
BinaryIO::WriteString(ofstream* fs, std::string string) {
	fs->write(string.c_str(), string.size() + 1);
}
void 
BinaryIO::WriteChars(ofstream* fs, std::string value) {
	WriteUInt32(fs, value.size() + 1);
	fs->write(value.c_str(), value.size() + 1);
}
void 
BinaryIO::WriteSignature(ofstream* fs, std::string value) {
	uint32_t* streamHeader = reinterpret_cast<uint32_t*>(&value);
	*streamHeader = ntohl(*streamHeader);
	WriteUInt32(fs, *streamHeader);
}

void 
BinaryIO::WriteUInt64(std::stringstream& ss, uint64_t value, bool swap) {
	if (swap)
		value = swapLongEndian(value);
	ss.write(reinterpret_cast<const char*>(&value), sizeof(uint64_t));
}
void 
BinaryIO::WriteUInt32(std::stringstream& ss, uint32_t value, bool swap) {
	if (swap)
		value = ntohl(value);
	ss.write(reinterpret_cast<const char*>(&value), sizeof(uint32_t));
}
void 
BinaryIO::WriteUInt16(std::stringstream& ss, uint16_t value) {
	ss.write(reinterpret_cast<const char*>(&value), sizeof(uint16_t));
}

void
BinaryIO::WriteSInt16(std::stringstream& ss, int16_t value) {
	ss.write(reinterpret_cast<const char*>(&value), sizeof(int16_t));
}

void 
BinaryIO::WriteByte(std::stringstream& ss, uint8_t value) {
	ss.write(reinterpret_cast<const char*>(&value), sizeof(uint8_t));
}

void
BinaryIO::WriteSInt8(stringstream& ss, int8_t value){
	ss.write(reinterpret_cast<const char*>(&value), sizeof(int8_t));
}

void 
BinaryIO::WriteBool(std::stringstream& ss, bool flag) {
	ss.write(reinterpret_cast<const char*>(&flag), sizeof(bool));
}
void 
BinaryIO::WriteInt32(std::stringstream& ss, int32_t value) {
	ss.write(reinterpret_cast<const char*>(&value), sizeof(int32_t));
}
void 
BinaryIO::WriteFloat(std::stringstream& ss, float value) {
	ss.write(reinterpret_cast<const char*>(&value), sizeof(float));
}

void 
BinaryIO::WriteString(std::stringstream& ss, const std::string& str, bool add_size_tag) {
	if (add_size_tag) {
		WriteChars(ss, str);
		return;
	}

	ss.write(str.c_str(), str.size() + 1);
}

void 
BinaryIO::WriteChars(std::stringstream& ss, const std::string& value) {
	WriteUInt32(ss, value.size() + 1);
	ss.write(value.c_str(), value.size() + 1);
}
void 
BinaryIO::WriteSignature(std::stringstream& ss, const std::string& value) {
	uint32_t streamHeader = *reinterpret_cast<const uint32_t*>(value.c_str());
	streamHeader = ntohl(streamHeader);
	WriteUInt32(ss, streamHeader);
}

void BinaryIO::align_binary_stream(std::stringstream& stream, int8_t alignment_value)
{
	while (stream.tellp() % alignment_value != 0) {
		WriteByte(stream, 0);
	}
}

void BinaryIO::align_binary_stream(char*& buffer, int8_t alignment_value) 
{
	while (uintptr_t(buffer) % alignment_value != 0) {
		WriteUInt8(buffer, 0);
	}
}

void BinaryIO::round_size(uint32_t& size, const int8_t value) {
	while (size % value != 0) {
		size++;
	}
}