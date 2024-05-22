#include "modelcereal.h"
#pragma once

namespace Wavefront 
{
	void WriteMeshToFile(std::ofstream& myfile, Mesh* mesh);

	enum Enum {
		POSITION  = 177619,
		TEXCOORD  = 5861255,
		NORMAL    = 5861213,
		INDEX     = 177603,
		OBJECT    = 177610,
		POLYGROUP = 177602,
	};

	class ObjFile
	{
	public:
		ObjFile(const char* path);
		bool load();
		const std::vector<Mesh>& meshes() const;
		Mesh* getMeshByGeo(const int numVerts);

	private:
		void alignZAxis(Mesh& mesh);

	private:
		void push_mesh(Mesh& mesh, std::vector<float>& verts, std::vector<float>& normals);
		void readPolygroup(Mesh& mesh, std::istringstream& ss, std::vector<float>& verts, std::vector<float>& normals);
		void readCoords(std::vector<float>& data, std::istringstream& ss);
		void readNormals(std::vector<float>& data, std::istringstream& ss);

	private:
		int m_meshCount;
		std::string m_path;
		std::vector<Mesh> m_meshes;
	};

}


