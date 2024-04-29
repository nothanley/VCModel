#include "meshstructs.h"
#include "MikkGen.h"

static void setFlatTangentBinormals(Mesh& mesh)
{
	for (int i = 0; i < mesh.numVerts; ++i){
		mesh.tangents.push_back(1.0f);
		mesh.tangents.push_back(1.0f);
		mesh.tangents.push_back(1.0f);
		mesh.tangents.push_back(0.0f);
		mesh.binormals.push_back(1.0f);
	}
}


void Mesh::calculateTangentsBinormals(const bool use_tangents)
{
	this->tangents.clear();
	this->binormals.clear();

	if (!use_tangents || uvs.empty()) {
		::setFlatTangentBinormals(*this);
		return;
	}

	MikkTCalc mikkcalculator(this);
	mikkcalculator.generate();
}

