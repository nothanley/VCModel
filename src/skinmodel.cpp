#include "skinmodel.h"
#include "BinaryIO.h"
#include "modelfile.h"
#include "winsock.h"
#include "meshtags.h"
#include "databuffers.h"
#include "wavefront.h"
using namespace BinaryIO;

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

void CSkinModel::free()
{
	/* debug function. delete me.*/
	delete this;
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
	/* Gets overall axis-aligned bounding box for model package */
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
