#include "wavefront.h"
#include <fstream>
#include <string>
#include <sstream>

using namespace Wavefront;

ObjFile::ObjFile(const char* path)
	: m_path(path)
{
}

static constexpr unsigned int str2int(const char* str, int h = 0) {
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

void ObjFile::alignZAxis(Mesh& mesh)
{
	for (int i = 0; i < mesh.vertices.size(); i+=3)
	{
		Vec3 vertex = {
			mesh.vertices[i],
			mesh.vertices[i+1],
			mesh.vertices[i+2] };

		mesh.vertices[i]   =         (vertex.x);
		mesh.vertices[i+1] = -1.0f * (vertex.y);
		mesh.vertices[i+2] = -1.0f * (vertex.z);
	}

	if (mesh.normals.size() != mesh.vertices.size())
		return;

	for (int i = 0; i < mesh.normals.size(); i += 3)
	{
		Vec3 normal = {
			mesh.normals[i],
			mesh.normals[i+1],
			mesh.normals[i+2] };
	
		mesh.normals[i]   =         (normal.x);
		mesh.normals[i+1] = -1.0f * (normal.y);
		mesh.normals[i+2] = -1.0f * (normal.z);
	}
}

void ObjFile::push_mesh(Mesh& mesh, std::vector<float>& verts, std::vector<float>& normals)
{
	mesh.vertices = verts;
	mesh.normals = normals;
	mesh.numVerts = verts.size() / 3;

	alignZAxis(mesh);
	m_meshes.push_back(mesh);
}

void ObjFile::readPolygroup(Mesh& mesh, std::istringstream& ss, std::vector<float>& verts, std::vector<float>& normals)
{
	if (!mesh.name.empty())
	{
		this->push_mesh(mesh, verts, normals);

		mesh = Mesh();
		verts.clear();
		normals.clear();
	}

	ss >> mesh.name;
	//printf("\n[OBJ Load] Read mesh: %s", mesh.name.c_str());
}

void ObjFile::readCoords(std::vector<float>& data, std::istringstream& ss)
{
	float x, y, z;
	ss >> x >> y >> z;

	data.push_back(x);
	data.push_back(y);
	data.push_back(z);
}

void ObjFile::readNormals(std::vector<float>& data, std::istringstream& ss)
{
	float nx, ny, nz;
	ss >> nx >> ny >> nz;

	data.push_back(nx);
	data.push_back(ny);
	data.push_back(nz);
}

const std::vector<Mesh>& ObjFile::meshes() const
{
	return m_meshes;
}

Mesh* ObjFile::getMeshByGeo(const int target_verts)
{
	for (Mesh& mesh : m_meshes)
		if (mesh.numVerts == target_verts)
			return &mesh;

	return nullptr;
}

bool ObjFile::load()
{
	std::ifstream file(m_path);
	if (!file.is_open())
		return false;

	Mesh mesh;
    std::string line;
	std::vector<float> verts, normals, texcoords;

	while (std::getline(file, line))
	{
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        int hash = str2int(prefix.c_str());
        switch (hash)
        {
			case Wavefront::OBJECT:
			case Wavefront::POLYGROUP: 
				readPolygroup(mesh, ss, verts, normals);
				break;
            case Wavefront::POSITION:
				readCoords(verts, ss);
				break;
            case Wavefront::NORMAL:
				readNormals(normals, ss);
				break;
            default:
				//printf("\nUnknown hash for %s with %d", line.c_str(), hash);
                break;
        }
	}

	this->push_mesh(mesh, verts, normals);
	return true;
}


void writeVerts(std::ofstream& myfile, Mesh* mesh) 
{
	myfile << "\n";
	int numVerts = mesh->vertices.size();
	for (int i = 0; i < numVerts; i += 3) {
		auto x = mesh->vertices.at(i);
		auto y = mesh->vertices.at(i + 1);
		auto z = mesh->vertices.at(i + 2);
		myfile << "\nv " << std::to_string(x) << " " << std::to_string(-y) << " " << std::to_string(-z);
	}
}

void writeNorms(std::ofstream& myfile, Mesh* mesh) 
{
	myfile << "\n";
	int numNorms = mesh->normals.size();
	int array_size = numNorms / mesh->numVerts;

	for (int i = 0; i < numNorms; i += array_size) {
		auto x = mesh->normals.at(i);
		auto y = mesh->normals.at(i + 1);
		auto z = mesh->normals.at(i + 2);
		myfile << "\nvn " << std::to_string(x) << " " << std::to_string(-y) << " " << std::to_string(-z);
	}
}

void writeUVs(std::ofstream& myfile, Mesh* mesh) 
{
	myfile << "\n";
	if (mesh->uvs.size() == 0)
		return;

	/* Export only default uv channel to .obj stream */
	auto defaultMap = mesh->uvs.front().map;

	int numUVs = defaultMap.size();
	for (int i = 0; i < numUVs; i += 2) {
		auto x = defaultMap.at(i);
		auto y = defaultMap.at(i + 1);

		x = (x < 0) ? -x : x;
		y = (y < 0) ? -y : y;
		y--;
		myfile << "\nvt " << std::to_string(x) << " " << std::to_string(-y);
	}
}

void writeFaces(std::ofstream& myfile, Mesh* mesh)
{
	myfile << "\n";
	myfile << "\ng " << mesh->name;
	for (auto& triangle : mesh->triangles) {
		int x = triangle[0] + 1;
		int y = triangle[1] + 1;
		int z = triangle[2]+ 1;

		myfile << "\nf " << std::to_string(y) << "/" << std::to_string(y) << "/" << std::to_string(y);
		myfile << " " << std::to_string(x) << "/" << std::to_string(x) << "/" << std::to_string(x);
		myfile << " " << std::to_string(z) << "/" << std::to_string(z) << "/" << std::to_string(z);
	}
}

void
Wavefront::WriteMeshToFile(std::ofstream& file, Mesh* mesh)
{
	file << "\n# CakeView OBJ Export\n";

	::writeVerts(file, mesh);
	::writeNorms(file, mesh);
	::writeUVs(file, mesh);
	::writeFaces(file, mesh);
}
