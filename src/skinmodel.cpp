#include "skinmodel.h"
#include "BinaryIO.h"
#include "modelfile.h"
#include "wavefront.h"
#include "meshencoder.h"
using namespace BinaryIO;

CSkinModel::CSkinModel() : CSerializedModel()
{
	m_parent = nullptr;
}

CSkinModel::CSkinModel(char* data, CModelContainer* pParent)
	: CSerializedModel(),
	m_parent(pParent)
{
	m_data = data;
	this->loadData();
}

CSkinModel::~CSkinModel()
{
	for (auto& mesh : m_meshes) {
		//printf("\n[CSkinModel] Deleting mesh: %s", mesh->material.name.c_str());
		delete mesh;
	}

	for (auto& bone : m_bones) {
		//printf("\n[CSkinModel] Deleting bone: %s", bone->name.c_str());
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

const BoundingBox CSkinModel::getAABBs() 
{
	if (m_meshes.empty())
		return BoundingBox();

	/* Iterate and compare each box for the highest/lowest coordinate values */
	BoundingBox totalBox = m_meshes.front()->bounds;
	for (auto& mesh : m_meshes) {

		BoundingBox& box = mesh->bounds;
		totalBox.maxX = (box.maxX > totalBox.maxX) ? box.maxX : totalBox.maxX;
		totalBox.maxY = (box.maxY > totalBox.maxY) ? box.maxY : totalBox.maxY;
		totalBox.maxZ = (box.maxZ > totalBox.maxZ) ? box.maxZ : totalBox.maxZ;

		totalBox.minX = (box.minX < totalBox.minX) ? box.minX : totalBox.minX;
		totalBox.minY = (box.minY < totalBox.minY) ? box.minY : totalBox.minY;
		totalBox.minZ = (box.minZ < totalBox.minZ) ? box.minZ : totalBox.minZ;
	}

	return totalBox;
}

