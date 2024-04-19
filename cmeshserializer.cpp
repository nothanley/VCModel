#include "cmeshserializer.h"
#include "cmodelserializer.h"
#include <skinmodel.h>
#include <BinaryIO.h>

using namespace BinaryIO;

CMeshSerializer::CMeshSerializer(CModelSerializer* parent)
	: m_parent(parent)
{
}

void CMeshSerializer::generateMeshBuffers(std::vector<StMeshBf>& buffers)
{
	const auto& meshes = m_parent->model()->getMeshes();

	for (auto& targetMesh : meshes) {
		StMeshBf meshbuffer;
		meshbuffer.mesh = targetMesh;

		serializeVertices(meshbuffer);
		serializeVertexNormals(meshbuffer);
		serializeTangents(meshbuffer);
		serializeBinormals(meshbuffer);
		serializeVertexColors(meshbuffer);
		serializeTexCoords(meshbuffer);
		serializeSkin(meshbuffer);
		serializeVertexRemap(meshbuffer);
		serializeBlendShapes(meshbuffer);
		serializeColorDict(meshbuffer);

		buffers.push_back(meshbuffer);
	}
}


void CMeshSerializer::serializeVertices(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_parent->table(), "POSITION", "R32_G32_B32", "float");

	/* Write vertex buffer */
	std::vector<float>& vertices = target.mesh->vertices;
	dataBf->stream.write((char*)vertices.data(), sizeof(float) * vertices.size());

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}

void CMeshSerializer::serializeVertexNormals(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_parent->table(), "NORMAL", "R8_G8_B8_A8", "snorm");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& normals = target.mesh->normals;

	for (int i = 0; i < normals.size(); i += 4) {
		// Convert to signed normals
		Vec3 normal{ normals[i], normals[i + 1], normals[i + 2] };
		normal.pack_values(1.0);
		normal *= 127;

		WriteSInt8(stream, normal.x);
		WriteSInt8(stream, normal.z);
		WriteSInt8(stream, normal.y);
		WriteSInt8(stream, 0);
	}

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}

void CMeshSerializer::serializeTangents(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_parent->table(), "TANGENT", "R8_G8_B8_A8", "snorm");

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

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}

void CMeshSerializer::serializeBinormals(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	dataBf->setHeader(m_parent->table(), "BINORMAL", "R8", "snorm");

	/* Write vertex normal buffer */
	auto& stream = dataBf->stream;
	std::vector<float>& binormals = target.mesh->binormals;

	for (int i = 0; i < target.mesh->numVerts; ++i) {
		Vec3 normal{ binormals[i] };
		normal.pack_values(1.0); // Convert to snorm
		normal *= 127;

		WriteSInt8(stream, normal.x);
	}

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}


void CMeshSerializer::serializeVertexColors(StMeshBf& target)
{
	for (auto& colorMap : target.mesh->colors)
	{
		auto dataBf = std::make_shared<StDataBf>();
		dataBf->setHeader(m_parent->table(), "COLOR", "R8_G8_B8_A8", "unorm");

		/* Write vertex normal buffer */
		auto& stream = dataBf->stream;
		std::vector<float>& map = colorMap.map;

		for (int i = 0; i < map.size(); i += 4) {
			Vec4 srgbColor{ map[i], map[i + 1], map[i + 2], map[i + 3] };

			WriteByte(stream, srgbColor.x);
			WriteByte(stream, srgbColor.y);
			WriteByte(stream, srgbColor.z);
			WriteByte(stream, srgbColor.w);
		}

		::align_binary_stream(dataBf->stream);
		target.buffers.push_back(dataBf);
	}
}

void CMeshSerializer::serializeTexCoords(StMeshBf& target)
{
	for (auto& channel : target.mesh->texcoords)
	{
		auto dataBf = std::make_shared<StDataBf>();
		dataBf->setHeader(m_parent->table(), "TEXCOORD", "R32_G32", "float");

		/* Write vertex normal buffer */
		auto& stream = dataBf->stream;
		stream.write((char*)channel.map.data(), sizeof(float) * channel.map.size());

		::align_binary_stream(dataBf->stream);
		target.buffers.push_back(dataBf);
	}
}

void CMeshSerializer::serializeSkin(StMeshBf& target)
{
	if (target.mesh->skin.indices.size() == 0)
		return;

	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "SKIN";

	/* Write index buffer */
	auto& skin = target.mesh->skin;
	WriteUInt16(stream, m_parent->model()->getNumBones() + 1);
	WriteUInt16(stream, skin.numWeights);

	/* Skin blendindice buffer */
	for (auto& index : skin.indices) {
		WriteUInt16(stream, index);
	}

	/* Skin blendweight buffer */
	for (auto& weight : skin.weights) {
		uint8_t unorm = weight * 255;
		WriteByte(stream, unorm);
	}

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
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

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}

void CMeshSerializer::serializeBlendShapes(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "BLENDSHAPES";

	/* todo: ...everything */
	int numMorphs = target.mesh->blendshapes.size();
	WriteUInt32(stream, 0);

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}

void CMeshSerializer::serializeColorDict(StMeshBf& target)
{
	auto dataBf = std::make_shared<StDataBf>();
	auto& stream = dataBf->stream;
	dataBf->container = "COLOR_DICT";

	WriteUInt32(stream, m_parent->indexOf("")); // Value is always const
	for (auto& colorMap : target.mesh->colors) {
		int colorMapNameIndex = 0;
		WriteUInt32(stream, colorMapNameIndex);
	}

	::align_binary_stream(dataBf->stream);
	target.buffers.push_back(dataBf);
}

