#include "skinmodel.h"
#include "BinaryIO.h"
#include "modelfile.h"
#include "wavefront.h"
using namespace BinaryIO;

CSkinModel::CSkinModel(const int* version)
	: m_data(nullptr),
	m_parent(nullptr),
	m_version(*version)
{
}

CSkinModel::CSkinModel(char* data, CModelContainer* pParent)
    : m_data(data),
	  m_parent(pParent)
{
	this->loadData();
}

CSkinModel::~CSkinModel()
{
	for (auto& mesh : m_meshes) {
		delete mesh;
	}

	for (auto& bone : m_bones) {
		delete bone;
	}
}

void
CSkinModel::loadData()
{
	printf("Loading VCModel v%x\n", m_parent->getVersion() );
	int numPacks = ReadUInt32(m_data);
	this->loadAxisBounds();
}

void
CSkinModel::loadAxisBounds()
{
	m_axisBox = BoundingBox{ /* Axis min-boundaries */ ReadFloat(m_data), ReadFloat(m_data), ReadFloat(m_data),
							 /* Axis max-boundaries */ ReadFloat(m_data), ReadFloat(m_data), ReadFloat(m_data) };
}

void
CSkinModel::saveObjFile(Mesh* mesh, const char* path)
{
    /* Create mesh out file path */
    std::ofstream file( path );
    if (!file.is_open()) return;

    /* If file is valid, create a wavefront 3d container */
    Wavefront::WriteMeshToFile(file, mesh);
    file.close();
}

void
CSkinModel::saveToObjFile(const char* path, bool split)
{
	for (auto& mesh : m_meshes) {
		saveObjFile(mesh, path);
	}

	return;
}
