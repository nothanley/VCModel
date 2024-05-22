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

CSkinModel::CSkinModel() : CSerializedModel(nullptr)
{
}

CSkinModel::CSkinModel(char* data, CModelContainer* parent)
	: CSerializedModel(parent)
{
	m_data = data;
}

CSkinModel::~CSkinModel()
{
	for (auto& mesh : m_meshes) {
		//printf("\n[CSkinModel] Deleting mesh: %s", mesh->name.c_str());
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

void CSkinModel::loadBuffer()
{
	if (m_data > m_parent->end() )
		throw std::runtime_error("Failed to parse mdl contents");

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
		//case GtPt:
			//this->loadGtPtData();
			//break;
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

static StMdlDataRef* findDataRef(std::vector<StMdlDataRef>& refs, const char* format, const Mesh* target_mesh)
{
	for (auto& ref : refs)
	{
		Mesh* mesh = ref.mesh;
		if (!mesh) continue;

		if (ref.format == format && mesh->numVerts == target_mesh->numVerts)
			return &ref;
	}

	return nullptr;
}

void CSkinModel::injectNormalBuffer(const std::vector<float>& normals, StMdlDataRef* dataRef)
{
	if (!dataRef || dataRef->data != "snorm" || dataRef->type != "R8_G8_B8_A8")
		return;

	const int array_width = 3;
	char* normalBf = (char*)dataRef->address;

	if (normals.size() / array_width != dataRef->mesh->numVerts) 
		return;

	for (int i = 0; i < normals.size(); i+=array_width)
	{
		Vec3 normal{ normals[i], normals[i+1], normals[i+2] };
		normal *= 128.0f;

		WriteSInt8(normalBf, normal.x);
		WriteSInt8(normalBf, normal.y);
		WriteSInt8(normalBf, normal.z);
		WriteSInt8(normalBf, 0);
	}
}

void CSkinModel::injectVertexBuffer(const std::vector<float>& verts, StMdlDataRef* dataRef)
{
	if (!dataRef || dataRef->data != "float" || dataRef->type != "R32_G32_B32")
		return;

	const char* address = (char*)dataRef->address;
	char* positionBf    = (char*)dataRef->address;

	for (auto& vert : verts){
		WriteFloat(positionBf, vert);
	}
}

bool
CSkinModel::injectObj(const char* path, const int index)
{
	if (index > m_meshes.size())
		return false;

	const auto& source_mesh = m_meshes[index];

	Wavefront::ObjFile file(path);
	if (!file.load() || file.meshes().empty())
		return false;

	Mesh* new_mesh = file.getMeshByGeo(source_mesh->numVerts);
	if (!new_mesh) return false;

	this->injectVertexBuffer( new_mesh->vertices, ::findDataRef(m_dataRefs, "POSITION", source_mesh) );
	this->injectNormalBuffer( new_mesh->normals,  ::findDataRef(m_dataRefs, "NORMAL",  source_mesh) );
	return true;
}

