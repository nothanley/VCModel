#include "MemoryReader/memoryreader.h"
#include "common.h"
#include "modelfile.h"
#include "meshtags.h"
#include "skinmodelpoly.h"
#include "yukes/yukesobj.h"

using namespace memreader;

CModelContainer::CModelContainer(const char* path, bool use_lightweight_loader)
	: m_sFilePath(path),
	  m_data(nullptr),
	  m_fileBf(nullptr),
	  m_isReady(false),
	  m_model(nullptr)
{
	m_loadType = use_lightweight_loader ? enModelDefs::LoadLightWeight : enModelDefs::LoadNormal;
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

void CModelContainer::refresh()
{
	try 
	{
		validateFile();
		CModelContainer::loadModel();
	}
	catch(...){
		std::cout << ("\nModel failed to reload.");
	}
}

void
CModelContainer::load() 
{
	if (!m_fileBf){
		this->m_fileBf   = SysCommon::readBinaryFile(m_sFilePath);
		this->m_fileSize = ::getDiskFileSize(m_sFilePath);
	}

	if (!m_fileBf)
		throw std::runtime_error("Could not read Model file.");

	this->validateFile();

	// Legacy format contains only the MDL! stream definition ...
	if (this->m_signature == MDL_MAGIC)
	{
		this->loadModel();
		return;
	}

	// Modern format combines all definitions into a single stream ...
	while (m_data < this->end())
	{
		char* stream  = m_data;
		uint32_t size = ReadUInt32(stream);
		uint32_t sig  = ReadUInt32(stream);

		switch (sig)
		{
			case MDL_MAGIC:
				this->readMdl();
				break;
			case CTG_MAGIC:
				this->readCtg();
				break;
			default:
				break;
		}

		m_data = stream + size + sizeof(uint32_t);
		memreader::align_binary_stream(m_data);
	}
}

void
CModelContainer::readYukes()
{
	//printf("Opening YUKES Model File: %s\n", m_sFilePath.c_str());
	this->m_model = std::make_shared<CYukesSkinModel>(m_data, this);
 }

void
CModelContainer::readCtg()
{
	uint32_t size = ReadUInt32(m_data);
	uint32_t sig  = ReadUInt32(m_data);

	if (size == NULL || (sig != CTG_MAGIC))
		throw std::runtime_error("Attempting to read contents of an invalid MDL container.");

	// todo: implement CTG reader
}

void
CModelContainer::readMdl()
{
	uint32_t size = ReadUInt32(m_data);
	uint32_t sig  = ReadUInt32(m_data);

	if (size == NULL || (sig != MDL_MAGIC))
		throw std::runtime_error("Attempting to read contents of an invalid MDL container.");

	m_data += sizeof(uint32_t);
	m_version   = ReadUInt32(m_data);

	this->loadModel();
}

void
CModelContainer::loadModel() 
{
	//printf("Opening Model File: %s\n", m_sFilePath.c_str());
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
		case MDL_VERSION_2_9:
			this->m_model = std::make_shared<CSkinModel_2_9>(m_data, this);
			break;
		case MDL_VERSION_2_15:
			this->m_model = std::make_shared<CSkinModel_2_15>(m_data, this);
			break;
		default:
			throw std::runtime_error("Attempting to read contents of an invalid MDL container.");
			break;
	}

	memreader::align_binary_stream(m_data);
}

void
CModelContainer::validateFile() 
{
	/* Initialize stream pointer*/
	this->m_data = m_fileBf;
	auto stream =  m_fileBf;

	/* Get file tag data */
	m_signature = ReadUInt32(m_data);
	m_version   = ReadUInt32(m_data);
	m_isReady   = (m_signature == MDL_MAGIC || m_signature == MCD_MAGIC);

	if (!m_isReady)
		throw std::runtime_error("Attempting to read contents of an invalid MCD container.");
}
