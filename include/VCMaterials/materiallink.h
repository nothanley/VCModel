#include <string>
#include <memory>
#pragma once

class CMaterialLibrary;
class CMaterialTemplates;
class CSkinModel;
struct StPropertyNode;
struct StMaterial;
struct Mesh;

class MaterialLinker
{
public:
	static void relinkMaterialGUID(StMaterial& material, const std::string& mtlsPath);

	static void relinkMaterialName(StMaterial& material);

	static StPropertyNode* findNode(StMaterial& material, const char* target_name);

	static std::string getNodeBaseMapId(StMaterial& material, StPropertyNode& node);

	static bool userHasTexture(const std::string& texture_map_name, const std::string& mtlsPath);

	static void updateMaterialAsset(StMaterial& material, StPropertyNode& node, const std::string& mtlsPath);

	static bool isWhitelistedMap(const std::string& texture_map_name);

	static void updateMaterialMap(StMaterial& material, StPropertyNode& node);

	static std::string formatTextureModelName(const std::string& texture_map_id);

	static StMaterial autoPopulateMaterial(const Mesh* mesh, CMaterialTemplates& presets );

	static void convertSkinModelToLibrary(const char* model_path, const char* json_path, std::shared_ptr<CMaterialLibrary>& library);

	static void convertSkinModelToLibrary(CSkinModel* model, const char* json_path, std::shared_ptr<CMaterialLibrary>& library);

};

