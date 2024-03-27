#include "modelfile.h"
#include "vcmodel.h"
#include <BinaryIO.h>
#include "meshtags.h"
#include "winsock.h"

using namespace BinaryIO;

VCModel::CContainer::CContainer(std::istream* stream)
    : fs(stream)
{
    Load();
}


VCModel::CContainer::CContainer(const char* path) 
	: m_sFilePath(path)
{
	Load();
//	fs->close();
}

//static
//uintptr_t GetFileBufferSize(std::filebuf* buffer)
//{
//	buffer->pubseekoff(0, std::ios::end);
//	std::streampos size = buffer->pubseekoff(0, std::ios::cur);
//	return uintptr_t(size);
//}

void
VCModel::CContainer::Load() {
	if (!fs)
		fs= new std::ifstream(this->m_sFilePath, ios::binary);
	if (!fs->good())
		throw std::runtime_error("Cannot read MDL stream.");

	CContainer::ValidateContainer();
	CContainer::ReadContents();
//	fs->close();
}

void
VCModel::CContainer::ReadContents() {
	if (!isOk)
		throw std::logic_error("Attempting to read contents of an invalid MDL container.");
		
	fs->seekg(0x4);
	m_fileVersion = BinaryIO::ReadUInt32(*fs);

	printf("Opening Model File: %s\n", m_sFilePath.c_str());

	m_pModel = new VCModel::CPackage(fs, this);
	return;

	try {
		m_pModel = new VCModel::CPackage(fs, this); }
	catch (...) {
		throw std::runtime_error("Failed to load VCModel file.");
	}
}

void
VCModel::CContainer::ValidateContainer() {
	fs->seekg(ios::beg);
	uint32_t signature = ReadUInt32(*fs);
//	this->m_fileSize = GetFileBufferSize(fs->rdbuf());

	// Validates type and version
	if (signature == MDL_MAGIC)
		this->isOk = true;

	/* Reset stream pointer */
	fs->seekg(ios::beg);
}

