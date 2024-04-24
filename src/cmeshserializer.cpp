#include "cmeshserializer.h"
#include <skinmodel.h>
#include <BinaryIO.h>
#include <crc32c/crc32c.h>
#include "meshshapes_serialize.h"

using namespace BinaryIO;
using namespace crc32c;
using bscompress = vCMeshShapeSerial;

const std::vector<std::string> STREAM_TABLE{
	"POSITION", "R32_G32_B32","float", "NORMAL", "R8_G8_B8_A8", "snorm",
	"TANGENT","BINORMAL","R8", "COLOR","unorm","TEXCOORD",
	"R32_G32","R16_G16_B16_A16","BLENDINDICES", "uint","BLENDWEIGHTS","R32_G32_B32_A32",""
};

int get_str_index(const std::vector<std::string>& table, const std::string& target)
{
	int index = -1;
	int numStrings = table.size();

	for (int i = 0; i < numStrings; i++) {
		const std::string& element = table.at(i);
		if (target == element)
			return i;
	}

	return -1;
}

void StDataBf::setHeader(const std::vector<std::string>& table, const char* data, const char* format, const char* type)
{
	/* Define stream container */
	this->container = data;
	WriteUInt32(stream, ::crc32c_lower(data));
	WriteUInt32(stream, ::crc32c_lower(format));
	WriteUInt32(stream, ::crc32c_lower(type));

	WriteUInt16(stream, ::get_str_index(table, data));
	WriteUInt16(stream, ::get_str_index(table, format));
	WriteUInt16(stream, ::get_str_index(table, type));
	::align_binary_stream(stream);
}

int CMeshSerializer::indexOf(const std::string& target) {
	return get_str_index(m_stringTable, target);
}


void CMeshSerializer::serializeVertices(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "POSITION", "R32_G32_B32", "float");

	/* Write vertex buffer */
	WriteUInt16(dataBf->stream, 0);
	std::vector<float>& vertices = target.mesh->vertices;
	dataBf->stream.write((char*)vertices.data(), sizeof(float) * vertices.size());

	target.data.push_back(dataBf);
}

void CMeshSerializer::serializeVertexNormals(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "NORMAL", "R8_G8_B8_A8", "snorm");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& normals = target.mesh->normals;

	for (int i = 0; i < normals.size(); i += 4) {
		// Convert to signed normals
		Vec3 normal{ normals[i], normals[i + 1], normals[i + 2] };
		normal.pack_values(1.0);
		normal *= 127;

		WriteSInt8(stream, normal.x);
		WriteSInt8(stream, normal.y);
		WriteSInt8(stream, normal.z);
		WriteSInt8(stream, 0);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void CMeshSerializer::serializeTangents(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "TANGENT", "R8_G8_B8_A8", "snorm");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& tangents = target.mesh->tangents;

	for (int i = 0; i < tangents.size(); i += 4) {
		// Convert to snorm
		Vec3 tangent{ tangents[i], tangents[i + 1], tangents[i + 2] };
		tangent.pack_values(1.0);
		tangent *= 127;

		WriteSInt8(stream, tangent.x);
		WriteSInt8(stream, tangent.y);
		WriteSInt8(stream, tangent.z);
		WriteSInt8(stream, 0);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void CMeshSerializer::serializeBinormals(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_stringTable, "BINORMAL", "R8", "snorm");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& binormals = target.mesh->binormals;

	for (int i = 0; i < target.mesh->numVerts; ++i) {
		Vec3 normal{ binormals[i] };
		normal.pack_values(1.0); // Convert to snorm
		normal *= 127;

		WriteSInt8(stream, normal.x);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

static inline uint8_t to_unorm(const float color) {
	return roundf(color * 255);
}

void CMeshSerializer::serializeVertexColors(StMeshBf& target)
{
	for (auto& colorMap : target.mesh->colors)
	{
		auto dataBf = std::make_shared<StDataBf>();
		dataBf->setHeader(m_stringTable, "COLOR", "R8_G8_B8_A8", "unorm");

		/* Write vertex normal buffer */
		auto& stream = dataBf->stream;
		std::vector<float>& map = colorMap.map;

		for (int i = 0; i < map.size(); i += 4) {
			Vec4 srgbColor{ map[i], map[i + 1], map[i + 2], map[i + 3] };

			WriteByte(stream, to_unorm(srgbColor.x));
			WriteByte(stream, to_unorm(srgbColor.y));
			WriteByte(stream, to_unorm(srgbColor.z));
			WriteByte(stream, to_unorm(srgbColor.w));
		}

		::align_binary_stream(stream);
		target.data.push_back(dataBf);
	}
}

void CMeshSerializer::serializeTexCoords(StMeshBf& target)
{
	for (auto& channel : target.mesh->texcoords)
	{
		auto dataBf = std::make_shared<StDataBf>();
		dataBf->setHeader(m_stringTable, "TEXCOORD", "R32_G32", "float");

		/* Write vertex normal buffer */
		auto& stream = dataBf->stream;
		stream.write((char*)channel.map.data(), sizeof(float) * channel.map.size());

		::align_binary_stream(stream);
		target.data.push_back(dataBf);
	}
}

void CMeshSerializer::serializeSkin(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "SKIN";

	/* Check empty skin buffer */
	auto& skin = target.mesh->skin;
	if (skin.indices.empty()) {
		WriteUInt32(stream, 0);
		target.data.push_back(dataBf);
		return;}

	/* Write index buffer */
	WriteUInt16(stream, m_model->getNumBones());
	WriteUInt16(stream, skin.numWeights);

	/* Skin blendindice buffer */
	for (auto& index : skin.indices) {
		WriteUInt16(stream, index);
	}
	::align_binary_stream(stream);

	/* Skin blendweight buffer */
	for (auto& weight : skin.weights) {
		uint8_t unorm = weight * 255;
		WriteByte(stream, unorm);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void CMeshSerializer::serializeVertexRemap(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "VERTEX_REMAP";

	/* use default vertex mapping */
	for (int i = 0; i < target.mesh->numVerts; i++) {
		WriteUInt32(stream, i);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void CMeshSerializer::serializeColorDict(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "COLOR_DICT";

	WriteUInt32(stream, indexOf("")); // Value is always const
	for (auto& colorMap : target.mesh->colors) {
		int colorMapNameIndex = 0;
		WriteUInt32(stream, colorMapNameIndex);
	}

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void CMeshSerializer::serializeUVDict(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "UV_DICT";

	/* Write texture uv channel names */
	for (auto& uvMap : target.mesh->texcoords) {
		WriteUInt32(stream, indexOf(uvMap.name));
	}

	/* Unknown 2024 values */
	WriteFloat(stream, 0.0f);
	WriteFloat(stream, 0.0f);

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}

void CMeshSerializer::generateStringTable()
{
	/* Push all bone names */
	auto bones = m_model->getBones();
	for (auto& bone : bones) {
		m_stringTable.push_back(bone->name);
	}

	/* Push all mesh+material names */
	auto meshes = m_model->getMeshes();
	for (auto& mesh : meshes) {
		m_stringTable.push_back(mesh->name);

		for (auto& uvMap : mesh->texcoords)
			m_stringTable.push_back(uvMap.name);

		for (auto& group : mesh->groups)
			m_stringTable.push_back(group.material.name);
	}

	/* Push all blendshape ids */
	for (auto& mesh : meshes) {
		for (auto& shape : mesh->blendshapes)
			m_stringTable.push_back(shape.name);
	}

	/* Push all stream types */
	for (auto& type : STREAM_TABLE) {
		m_stringTable.push_back(type);
	}
}


static void createShapeVertexBuffer(std::stringstream& indexBuffer, std::stringstream& weightBuffer,
	const Matrix3& deltaMatrix, Mesh* mesh, const StBlendShape& shape, uint32_t& offset)
{
	uint32_t indexMask = 0; // stores 'is morphed' status for 32 vertices
	uint8_t size = 0;

	for (int i = 0; i < mesh->numVerts; i++) {
		Vec3 original_vertex   = bscompress::getVertex(i * 3, mesh->vertices);
		Vec3 blendshape_vertex = bscompress::getVertex(i * 3, shape.vertices);

		if (original_vertex != blendshape_vertex) {
			bscompress::setBit( indexMask, (i % 0x20) ); // update index mask @ vertex index
			bscompress::updateShapeVtxWeightBuffer(weightBuffer, deltaMatrix, original_vertex, blendshape_vertex);
			size++;
		}

		if ((i + 1) % 0x20 == 0) // Update mask & stream every 32 indices
			bscompress::updateShapeVtxIndexBuffer(indexBuffer, indexMask, offset, size);
	}

	/* Process remaining vertices */
	if (mesh->numVerts % 0x20 != 0)
		bscompress::updateShapeVtxIndexBuffer(indexBuffer, indexMask, offset, size);
}

static inline uint32_t getIndexTableSize(int numShapes, int numVerts) {
	uint32_t completeSets	  = numVerts / 0x20;
	uint32_t remainingIndices = numVerts % 0x20;
	uint32_t numDataSets = completeSets + (remainingIndices > 0 ? 1 : 0);
	return numShapes * numDataSets * 8;
}

static void createVertexMorphs(std::stringstream& stream, Mesh* mesh)
{
	/* Generate delta matrix for overall blendshape coords */
	uint32_t weightOffset = getIndexTableSize(mesh->blendshapes.size(), mesh->numVerts);
	Matrix3 shapeDeltaMat = bscompress::getBlendShapePrecisionMatrix(mesh);
	std::stringstream indexBuffer, weightBuffer;
	for (auto& shape : mesh->blendshapes) {
		createShapeVertexBuffer(indexBuffer, weightBuffer, shapeDeltaMat, mesh, shape, weightOffset);
	}

	/* Write shape vertex + weight stream */
	WriteUInt32(stream, mesh->numVerts);
	bscompress::writeDeltaMatrix(stream, shapeDeltaMat);
	WriteUInt32(stream, indexBuffer.str().size() + weightBuffer.str().size() ); // Index table size
	stream.write( indexBuffer.str().data(),  indexBuffer.str().size()  );
	stream.write( weightBuffer.str().data(), weightBuffer.str().size() );
}

void CMeshSerializer::writeMeshShapes(std::stringstream& stream, Mesh* mesh)
{
	for (auto& shape : mesh->blendshapes)
		WriteUInt16(stream, indexOf(shape.name)); // Add all shape names to stream

	::align_binary_stream(stream);
	::createVertexMorphs(stream, mesh); // creates compressed blendshape stream
}

void CMeshSerializer::serializeBlendShapes(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "BLENDSHAPES";

	/* Serialize all blendshapes */
	Mesh* mesh = target.mesh;
	int numMorphs = mesh->blendshapes.size();
	WriteUInt32(stream, numMorphs);
	//printf("\nMesh '%s' has total shapes: %d", mesh->name.c_str(), mesh->blendshapes.size());

	/* Create shape key weight buffer */
	if (!mesh->blendshapes.empty())
		writeMeshShapes(stream, mesh);

	::align_binary_stream(stream);
	target.data.push_back(dataBf);
}



