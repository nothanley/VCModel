#include "wavefront.h"
#include <fstream>

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
		int x = triangle.x + 1;
		int y = triangle.y + 1;
		int z = triangle.z + 1;

		myfile << "\nf " << std::to_string(y) << "/" << std::to_string(y) << "/" << std::to_string(y);
		myfile << " " << std::to_string(x) << "/" << std::to_string(x) << "/" << std::to_string(x);
		myfile << " " << std::to_string(z) << "/" << std::to_string(z) << "/" << std::to_string(z);
	}
}

void
Wavefront::WriteMeshToFile(std::ofstream& file, Mesh* mesh)
{
    file << "\n# CakeView OBJ Export\n";

	writeVerts(file, mesh);
	writeNorms(file, mesh);
	writeUVs(file, mesh);
	writeFaces(file, mesh);
}
