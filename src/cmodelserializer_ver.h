#include <cmodelserializer.h>
#pragma once

class CModelSerializer_2_5 : public CModelSerializer
{
public:
	CModelSerializer_2_5(CSkinModel* target) : CModelSerializer(target)
	{
	}

private:

};

class CModelSerializer_2_8 : public CModelSerializer
{
public:
	CModelSerializer_2_8(CSkinModel* target) : CModelSerializer(target)
	{
	}

private:

};

class CModelSerializer_2_9 : public CModelSerializer
{
public:
	CModelSerializer_2_9(CSkinModel* target) : CModelSerializer(target)
	{

	}

protected:
	virtual void createMCDBuffer();

protected: 
	static void doJigBoneCheck(std::vector<RigBone*>& bones);

protected:
	void serialize() override;
	void formatFile() override;
	void writeDataBuffer(std::ofstream& fs, const StModelBf& data) override;
	void createModelBuffer() override;
	void createBoneBuffer() override;

protected:
	uint32_t getBoneBufferSize(const std::vector<RigBone*>& bones) override;
	void     writeMeshBuffer(char*& buffer, const StMeshBf& meshBuffer) override;
	void     serializeVertices(StMeshBf& target) override;
	uint32_t getMeshBufferDefSize(std::vector<StMeshBf>& meshbuffers) override;
	void     generateMeshBuffers(std::vector<StMeshBf>& buffers) override;
};


class CModelSerializer_2_15 : public CModelSerializer_2_9
{
public:
	CModelSerializer_2_15(CSkinModel* target) : CModelSerializer_2_9(target)
	{}

protected:
	void serialize() override;

protected:
	void createModelBuffer() override;
	void createMaterialBuffer() override;
	void writeMaterialGroupBuffer(char*& buffer, int meshIndex) override;
	void writeUvDictTail(std::stringstream& stream, Mesh* mesh) override;

protected:
	uint32_t getMtlBufferSize(const std::vector<Mesh*>& meshes) override;
	void updateIndexBufferSize(uint32_t& size, const Mesh* mesh) override;

protected:
	void serializeVertices(StMeshBf& target);
	void serializeVertexNormals(StMeshBf& target);
	void serializeTangents(StMeshBf& target);

};


 
