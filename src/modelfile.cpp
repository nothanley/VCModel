#include <BinaryIO.h>
#include "common.h"
#include "modelfile.h"
#include "meshtags.h"
#include "skinmodelpoly.h"
using namespace BinaryIO;

CModelContainer::CModelContainer(const char* path, bool use_lightweight_loader)
	: m_sFilePath(path),
	  m_data(nullptr),
	  m_fileBf(nullptr),
	  m_isReady(false),
	  m_model(nullptr)
{
	m_loadType = use_lightweight_loader ? enModelDefs::LoadLightWeight : enModelDefs::LoadNormal;

	LoadFile();
}

CModelContainer::CModelContainer(char* data, const size_t size, bool use_lightweight_loader)
  : m_sFilePath(""),
	m_data(nullptr),
	m_fileBf(data),
	m_isReady(false),
	m_model(nullptr),
	m_fileSize(size)
{
	m_loadType = use_lightweight_loader ? enModelDefs::LoadLightWeight : enModelDefs::LoadNormal;

	LoadFile();
}

int CModelContainer::getLoadType() {
	return m_loadType;
}

CModelContainer::~CModelContainer() 
{
    if (m_fileBf)
        delete m_fileBf;
}

void
CModelContainer::free_model()
{
	if (this->m_model) {
		m_model->~CSkinModel();
		m_model = nullptr;
	}
}

static const size_t getDiskFileSize(const std::string& filePath) 
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filePath << std::endl;
		return -1; // Return -1 to indicate error
	}

	std::streamsize fileSize = file.tellg();
	file.close();

	return fileSize;
}

void CModelContainer::reload()
{
	try 
	{
		std::cout << ("\nModel loading....");
		ValidateContainer();
		CModelContainer::ReadContents();
		std::cout << ("\nModel reloaded");
	}
	catch(...){
		std::cout << ("\nModel failed to reload.");
	}
}

void
CModelContainer::LoadFile() 
{
	if (!m_fileBf){
		this->m_fileBf   = SysCommon::readBinaryFile(m_sFilePath);
		this->m_fileSize = ::getDiskFileSize(m_sFilePath);
	}

	if (!m_fileBf)
		throw std::runtime_error("Could not read MDL file.");

	/* load skin model object */
	CModelContainer::ValidateContainer();

	try {
		CModelContainer::ReadContents();
	}
	catch(...){
		throw std::runtime_error("Could not read MDL file.");
	}
}

void
CModelContainer::ReadContents() 
{
	if (!m_isReady)
		throw std::runtime_error("Attempting to read contents of an invalid MDL container.");

	printf("Opening Model File: %s\n", m_sFilePath.c_str());
	switch (m_version)
	{
		case MDL_VERSION_1_1:
			this->m_model = std::make_shared<CSkinModel_1_1>(m_data, this);
			break;
		case MDL_VERSION_2_0:
			this->m_model = std::make_shared<CSkinModel_2_0>(m_data, this);
			break;
		case MDL_VERSION_2_5:
			this->m_model = std::make_shared<CSkinModel_2_5>(m_data, this);
			break;
		case MDL_VERSION_2_8:
			this->m_model = std::make_shared<CSkinModel_2_8>(m_data, this);
			break;
		default:
			throw std::runtime_error("Attempting to read contents of an invalid MDL container.");
			break;
	}
}

//	case MDL_VERSION_1_1:
//		this->m_model = new CSkinModel_1_1(m_data, this);
//		break;
//	case MDL_VERSION_2_0:
//		this->m_model = new CSkinModel_2_0(m_data, this);
//		break;
//	case MDL_VERSION_2_5:
//		this->m_model = new CSkinModel_2_5(m_data, this);
//		break;
//	case MDL_VERSION_2_8:
//		this->m_model = new CSkinModel_2_8(m_data, this);

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

