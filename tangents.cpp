#include "meshstructs.h"
#include "BinaryIO.h"

using namespace BinaryIO;

static Matrix4x3 getDeltas(const Mesh& mesh, const UVMap& uv_map, const Triangle& tri)
{
	const Vec3 v1 = mesh.vertex(tri.x);
	const Vec3 v2 = mesh.vertex(tri.y);
	const Vec3 v3 = mesh.vertex(tri.z);

	const Vec2 w1 = uv_map.texcoord(tri.x);
	const Vec2 w2 = uv_map.texcoord(tri.y);
	const Vec2 w3 = uv_map.texcoord(tri.z);
	return Matrix4x3{ (v2 - v1) , (v3 - v1) , (w2 - w1) , (w3 - w1) };
}

static Vec3 getFaceSignVec(const Matrix4x3& mat, const float& r)
{
	Vec3 sign{ r * (mat.w.y * mat.x.x - mat.z.y * mat.y.x) ,
               r * (mat.w.y * mat.x.y - mat.z.y * mat.y.y) ,
               r * (mat.w.y * mat.x.z - mat.z.y * mat.y.z)   };

	sign.normalize();
	return sign;
}

static Vec3 getFaceTanVec(const Matrix4x3& mat, const float& r)
{
	Vec3 tan{ r * (mat.z.x * mat.y.x - mat.w.x * mat.x.x) ,
              r * (mat.z.x * mat.y.y - mat.w.x * mat.x.y) ,
              r * (mat.z.x * mat.y.z - mat.w.x * mat.x.z)  };

	tan.normalize();
	return tan;
}

static void setVertexFaceVector(std::vector<Vec3>& data, const Vec3& vector, const Triangle& tri)
{
	data[tri.x] += vector;
	data[tri.y] += vector;
	data[tri.z] += vector;
}

static void calculateFaceTangents(std::vector<Vec3>& tan, std::vector<Vec3>& bin, const Mesh& mesh)
{
	auto& uvMap = mesh.uvs.front();
	tan.resize(mesh.numVerts);
	bin.resize(mesh.numVerts);

	for (auto& tri : mesh.triangles){
		Matrix4x3 mat = getDeltas(mesh, uvMap, tri);
		float r = 1.0f / (mat.z.x * mat.w.y - mat.w.x * mat.z.y);

		::setVertexFaceVector(tan , getFaceTanVec(mat, r)  , tri);
		::setVertexFaceVector(bin , getFaceSignVec(mat, r) , tri);
	}
}

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

static void calculateVertexTanBinormals(std::vector<Vec3>& tan, std::vector<Vec3>& bin, Mesh& mesh)
{
	for (int i = 0; i < mesh.numVerts; i++)
	{
		const Vec3 n = mesh.normal(i);
		Vec3& t = tan[i];
		Vec3& b = bin[i];

		if ( t.null() || b.null() )
		{
			t.x = n.y;
			t.y = n.z;
			t.z = n.x;
			b = Vec3::cross(n, t);
		}
		else
		{
			t = (t - n * Vec3::dot(n, t));
			t.normalize();

			b = (b - n * Vec3::dot(n, b));
			b = (b - t * Vec3::dot(t, b));
			b.normalize();
		}
	}
}

static void unpack_vector_to_array(const std::vector<Vec3>& data, 
	std::vector<float>& dst, bool pack_single=false)
{
	for (auto& vec : data)
	{
		if (pack_single){
			dst.push_back(vec.x);
		} else {
			Vec3 test = vec;

			test.x = int8_t(test.x * 127);
			test.y = int8_t(test.y * 127);
			test.z = int8_t(test.z * 127);
			
			test.x /= 127;
			test.y /= 127;
			test.z /= 127;

			dst.push_back(test.x);
			dst.push_back(test.y);
			dst.push_back(test.z);
			dst.push_back(0.0f);
		}
	}
}

void Mesh::calculateTangentsBinormals()
{
	//tangents.clear();
	//binormals.clear();

	//if (uvs.empty()) {
		//::setFlatTangentBinormals(*this);
		//return;
	//}

	auto& srcTans = tangents;
	auto& stcBins = binormals;
	std::vector<float> testTangents, testBinormals;

	/* generate face uv tangent values */
	std::vector<Vec3> tan, bin;
	::calculateFaceTangents(tan, bin, *this);
	::calculateVertexTanBinormals(tan, bin, *this);

	/* update tangents and binormals */
	::unpack_vector_to_array(tan, testTangents);
	::unpack_vector_to_array(bin, testBinormals, true);


 	printf("");
}

