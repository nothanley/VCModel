#include "skinmodel.h"
#include "BinaryIO.h"
#include "modelfile.h"
#include "wavefront.h"
#include "meshencoder.h"
#include <meshtags.h>

#include "materialfile.h"
#include "materialgen.h"
#include "materiallibrary.h"
#include "common.h"

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

void CSkinModel::loadBuffer()
{
	uint32_t type = ReadUInt32(m_data);
	uint32_t size = ReadUInt32(m_data);
	char* nextBfPtr = m_data + size;

	// Handle stream
	switch (type)
	{
		case TEXT:
			this->loadStringTable();
			break;
		case BONE:
			this->loadModelBones(size);
			break;
		case AtPt:
			this->loadAttachPtData();
			break;
		case MTL_:
			this->loadMaterials();
			break;
		case MBfD:
			this->loadMeshes();
			break;
		case LODs:
			this->loadLods();
			break;
		case END:
			return;
		default:
			break;
	}

	//Iterate through model structure
	m_data = nextBfPtr;
	loadBuffer();
}

static StPropertyNode* get_color_node(StMaterial* mat)
{
	if (!mat)
		return nullptr;

	for (auto& node : mat->nodes){
		if (node.name == "colorMap")
			return &node;
	}
	return nullptr;
}


void CSkinModel::linkMaterialsFile(const char* model_path)
{
	std::string mtlsPath = CMaterialGen::get_mtls_path(model_path);
	CMaterialFile mtlsFile( mtlsPath.c_str() );

	try {
		mtlsFile.load();
	}
	catch (...) {
		return;
	}

	auto library = mtlsFile.getLibrary();
	if (library->numMaterials() == 0) return;

	for (auto& mesh : m_meshes)
		for (auto& group : mesh->groups)
		{
			auto& group_id = group.material.name;
			int index = library->indexOf( group_id.c_str() );

			if (index == -1) {
				std::string prefix = SysCommon::split(group_id, ":").front();
				index = library->indexOf( prefix.c_str() );
			}

			StMaterial* mat = (index != -1) ? library->at(index) : nullptr;
			StPropertyNode* node = get_color_node(mat);
			group.material.color_map = (node) ? variant_string(node->value) : "";
		}
}


