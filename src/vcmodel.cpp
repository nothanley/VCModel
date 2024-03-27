#include "vcmodel.h"
#include "BinaryIO.h"
#include "modelfile.h"
#include "winsock.h"
#include "meshtags.h"
#include "databuffers.h"
#include "wavefront.h"

using namespace BinaryIO;
using namespace VCModel;

VCModel::CPackage::CPackage(std::istream* fs, VCModel::CContainer* pParent)
    : m_pDataStream(fs),
      m_pParent(pParent)
{

	VCModel::CPackage::validate();
	if ( !this->isOk() ) return;

	this->loadFile();
}

VCModel::CPackage::~CPackage()
{
	for (auto& mesh : m_meshes) {
		delete mesh;
	}
	for (auto& bone : m_bones) {
		delete bone;
	}
}

void
VCModel::CPackage::validate()
{

	m_type = m_pParent->getVersion();
	m_version =	 /* MAJOR */  (m_type >> 0x4) + /* MINOR */ ((m_type & 0xF) * 0.1f);

	switch (m_type)
	{
		case MDL_VERSION_2_5:
			this->bIsValidModel = true;
			break;
		case MDL_VERSION_2_8:
			this->bIsValidModel = true;
			break;
		default:
			printf("Unsupported model format %.1f\n", m_version);
			break;
	}

}

void
VCModel::CPackage::loadFile() 
{
	printf("Loading VCModel v%.1f\n", m_version);
	int numPacks = _U32;

	this->loadAxisBounds();
	this->loadBuffer();
}

void
VCModel::CPackage::loadAxisBounds()
{
	/* Gets overall axis-aligned bounding box for model package */
	m_axisBox = BoundingBox{ /* Axis min-boundaries */ _FLOAT, _FLOAT, _FLOAT,
							 /* Axis max-boundaries */ _FLOAT, _FLOAT, _FLOAT};
}

void 
VCModel::CPackage::loadBuffer()
{
	uint32_t type = ReadUInt32(*m_pDataStream);
	uint32_t size = ReadUInt32(*m_pDataStream);
	uintptr_t address = m_pDataStream->tellg();

	// Read data stream to new buffer
	char* stream = new char[size];
	m_pDataStream->read(stream, size);

	// Handle specified stream
	switch (type) {
		case TEXT:
			CDataBuffer::getStringTable(stream, address, m_stringTable);
			break;
		case BONE:
			CDataBuffer::getModelBones(stream, size, m_bones, m_stringTable, m_type);
			break;
		case MTL:
			CDataBuffer::getMaterials(stream, size, m_materials, m_stringTable);
			break;
		case MBfD:
			CDataBuffer::getMeshes(stream, size, m_meshes, m_stringTable);
			break;
		case LODs:
			CDataBuffer::getLods(stream, size, m_meshes, m_materials, m_stringTable);
			break;
		default:
			break;
	}

	/* Free memory */
	delete[] stream;

	//Iterate through model structure
	m_pDataStream->seekg(uintptr_t(address) + uintptr_t(size));
	if ( m_pDataStream->tellg() > (ios_base::end - ios_base::beg) )
		loadBuffer();
}



void
VCModel::CPackage::saveObjFile(Mesh* mesh, const char* path)
{
    /* Create mesh out file path */
    std::ofstream file( path );
    if (!file.is_open()) return;

    /* If file is valid, create a wavefront 3d container */
    Wavefront::WriteMeshToFile(file, mesh);
    file.close();
//    printf("\n\nSaved Object To: %s\n", path);
}

void
VCModel::CPackage::saveToObjFile(const char* path, bool split)
{
    if (true)
    {
        for (auto& mesh : m_meshes) {
            saveObjFile(mesh, path);
        }

        return;
    }
}
