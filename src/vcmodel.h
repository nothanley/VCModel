#include <fstream>
#include <istream>
#include <vector>
#include <string>
#include "databuffers.h"
#pragma once

struct RigBone;
struct Material;
struct BoundingBox;

namespace VCModel {

	class CContainer;
	class CPackage
	{
	public:
        CPackage(std::istream* fs, CContainer* pParent);
		~CPackage();

    public:
         static void saveObjFile(Mesh* mesh, const char* path);

	public:
		bool isOk() { return bIsValidModel; }
        std::vector<Mesh*> getMeshes() const  { return m_meshes; }
        int getNumMeshes(){ return m_meshes.size(); }
        int getNumBones(){ return m_bones.size(); }
        int getNumMats(){ return m_materials.size(); }
		void saveToObjFile(const char* path, bool split=true);

	private:
		void validate();
		void loadFile();

	private:
		void loadAxisBounds();
		void loadBuffer();

	private:
		std::vector<Mesh*> m_meshes;
		std::vector<RigBone*> m_bones;
		std::vector<std::string> m_stringTable;
		std::vector<Material> m_materials;
		BoundingBox m_axisBox;

	private:
		bool bIsValidModel = false;
		float m_version;
		int m_type;
        std::istream* m_pDataStream = nullptr;
		CContainer* m_pParent = nullptr;

	};

}


