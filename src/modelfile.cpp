#include "modelfile.h"
#include <BinaryIO.h>
#include "meshtags.h"
#include "winsock.h"
#include "common.h"
#include "skinmodelpoly.h"

using namespace BinaryIO;

CModelContainer::CModelContainer(const char* path)
	: m_sFilePath(path),
	  m_data(nullptr),
	  m_fileBf(nullptr),
	  m_isReady(false),
	  m_model(nullptr)
{
	LoadFile();
}

CModelContainer::~CModelContainer() 
{
	if (m_fileBf)
		delete m_fileBf;
}

void
CModelContainer::LoadFile() {
	m_fileBf = VCFile::Common::readBinaryFile(m_sFilePath);
	if (!m_fileBf)
		throw std::runtime_error("Cannot read MDL file.");

	CModelContainer::ValidateContainer();
	CModelContainer::ReadContents();
}

void
CModelContainer::ReadContents() {
	if (!m_isReady)
		throw std::logic_error("Attempting to read contents of an invalid MDL container.");

	printf("Opening Model File: %s\n", m_sFilePath.c_str());
	switch (m_version)
	{
		case MDL_VERSION_2_5:
			this->m_model = new CSkinModel_2_5(m_data, this);
			break;
		case MDL_VERSION_2_8:
			this->m_model = new CSkinModel_2_8(m_data, this);
			break;
		default:
			printf("Unsupported MDL version.\n");
			break;
	}
}

void
CModelContainer::ValidateContainer() 
{
	/* Initialize stream pointer*/
	uint32_t signature;
	this->m_data = m_fileBf;

	/* Get file tag data */
	signature  = ReadUInt32(m_data);
	m_version  = ReadUInt32(m_data);
	m_isReady  = (signature == MDL_MAGIC);
}

