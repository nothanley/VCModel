#include "materialgen.h"
#include "skinmodel.h"
#include <VCMaterials/mtllibraryserializer.h>
#include <VCMaterials/materiallink.h>
#include <Windows.h>
#include "common.h"

CMaterialGen::CMaterialGen(CSkinModel* model, const char* templates_name)
	: m_presetsPath(templates_name),
	m_model(model)
{
	m_presetsPath = this->expand_path(templates_name);
}

void CMaterialGen::save(const char* path)
{
	std::string full_path = CMaterialGen::get_mtls_path(path);
	this->setup_library();

	printf( "\nPresets path is: %s" , m_presetsPath.c_str() );
	if (!m_materials || m_materials->numMaterials() == 0)
		return;

	CMtlLibrarySerializer serializer(m_materials);
	serializer.save( full_path.c_str() );
}


void CMaterialGen::setup_library()
{
	m_materials = std::make_shared<CMaterialLibrary>();

	try {
		MaterialLinker::convertSkinModelToLibrary(m_model, m_presetsPath.c_str(), m_materials);
	}
	catch (...) {
		printf("\nFailed to convert materials");
	}
}


std::string CMaterialGen::get_mtls_path(const std::string model_path) 
{
	size_t lastDotPos = model_path.find_last_of(".");
	if (lastDotPos != std::string::npos) {
		std::string baseName = model_path.substr(0, lastDotPos);
		return baseName + "." + "mtls";
	}
	else { // If there's no existing extension, simply append the new extension
		return model_path + "." + "mtls";
	}
}

static std::string GetDllDirectory()
{
	TCHAR path[2048];
	GetModuleFileName(GetModuleHandle(("vcmodel.dll")), path, 2048);
	std::ostringstream file;
	file << path;

	return SysCommon::get_parent_folder(file.str());
}

std::string CMaterialGen::expand_path(const std::string& relativePath)
{
	return  GetDllDirectory() + "/" + relativePath;
}
