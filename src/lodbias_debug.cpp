#include "lodbias_debug.h"
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

namespace LodBiasDebug
{
	static std::vector<FileLodStats> s_records;

	static uint64_t make_edge_key(uint32_t a, uint32_t b)
	{
		uint32_t lo = (a < b) ? a : b;
		uint32_t hi = (a < b) ? b : a;
		return (uint64_t(hi) << 32) | uint64_t(lo);
	}

	static void compute_edge_stats(const Mesh* mesh,
		uint32_t& edgeCount,
		float& maxEdge,
		float& avgEdge,
		float& minEdge,
		float& medianEdge)
	{
		edgeCount = 0;
		maxEdge = avgEdge = minEdge = medianEdge = 0.0f;
		if (!mesh || mesh->triangles.empty()) return;

		std::unordered_set<uint64_t> edges;
		edges.reserve(mesh->triangles.size() * 3);
		std::vector<float> lengths;
		lengths.reserve(mesh->triangles.size() * 3);

		auto push_edge = [&](uint32_t a, uint32_t b)
		{
			uint64_t key = make_edge_key(a, b);
			if (!edges.insert(key).second) return;

			Vec3 va = mesh->vertex(static_cast<int>(a));
			Vec3 vb = mesh->vertex(static_cast<int>(b));
			Vec3 d  = va - vb;
			float len = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
			lengths.push_back(len);
		};

		for (const auto& tri : mesh->triangles)
		{
			push_edge(tri[0], tri[1]);
			push_edge(tri[1], tri[2]);
			push_edge(tri[2], tri[0]);
		}

		if (lengths.empty()) return;

		edgeCount = static_cast<uint32_t>(lengths.size());
		maxEdge = *std::max_element(lengths.begin(), lengths.end());
		minEdge = *std::min_element(lengths.begin(), lengths.end());

		double sum = 0.0;
		for (float v : lengths) sum += v;
		avgEdge = static_cast<float>(sum / lengths.size());

		std::sort(lengths.begin(), lengths.end());
		size_t mid = lengths.size() / 2;
		medianEdge = (lengths.size() % 2 == 1)
			? lengths[mid]
			: (lengths[mid - 1] + lengths[mid]) * 0.5f;
	}

	void record(const Mesh* mesh,
		float biasA,
		float biasB,
		std::vector<LodEntry> lods)
	{
		FileLodStats stat{};
		stat.mesh = mesh;
		stat.biasA = biasA;
		stat.biasB = biasB;
		stat.lods = std::move(lods);
		s_records.push_back(stat);
	}

	struct Candidate
	{
		std::string name;
		double meanRelErrA = 0.0;
		double meanRelErrB = 0.0;
		double scaleA = 0.0;
		double scaleB = 0.0;
	};

	static double rel_err(double expected, double actual)
	{
		if (expected == 0.0) return (actual == 0.0) ? 0.0 : 1.0;
		return std::abs(expected - actual) / std::abs(expected);
	}

	void report()
	{
		if (s_records.empty())
		{
			printf("\n[LodBiasDebug] No records captured.\n");
			return;
		}

		printf("\n[LodBiasDebug] Analyzing %zu meshes", s_records.size());

		const char* csvPath = "C:/Users/brian/Desktop/lodbias.csv";
		std::ofstream csv(csvPath);
		if (csv.is_open())
		{
			csv << "meshIndex,biasA,biasB,numVerts,numTris,boundsRadius,"
				<< "edge0,max0,avg0,min0,med0,"
				<< "edge1,max1,avg1,min1,med1\n";
		}

		printf("\n[LodBiasDebug] Per-mesh summary:");
		int meshIndex = 0;
		for (const auto& r : s_records)
		{
			if (!r.mesh) continue;

			const bool validMesh = r.mesh
				&& !r.lods.empty()
				&& r.lods[0].edgeCount > 0
				&& !r.mesh->triangles.empty()
				&& r.mesh->numVerts > 0
				&& r.mesh->numVerts < 10000000;

			uint32_t calcEdges = 0;
			float calcMax = 0, calcAvg = 0, calcMin = 0, calcMed = 0;
			if (validMesh) {
				compute_edge_stats(r.mesh, calcEdges, calcMax, calcAvg, calcMin, calcMed);
			}

			const size_t triCount = r.mesh->triangles.size();
			const LodEntry lod0 = r.lods.empty() ? LodEntry{} : r.lods[0];
			const double biasA_edge = (lod0.edgeCount > 0) ? (r.biasA * lod0.edgeCount) : 0.0;
			const double biasB_edge = (lod0.edgeCount > 0) ? (r.biasB * lod0.edgeCount) : 0.0;
			const double biasA_edge_avg = (lod0.edgeCount > 0 && lod0.avgEdge > 0.0f) ? (r.biasA * lod0.edgeCount * lod0.avgEdge) : 0.0;
			const double biasB_edge_avg = (lod0.edgeCount > 0 && lod0.avgEdge > 0.0f) ? (r.biasB * lod0.edgeCount * lod0.avgEdge) : 0.0;

			printf("\n  [%d] lods=%zu biasA=%g biasB=%g edges=%u max=%g avg=%g min=%g med=%g verts=%u tris=%zu radius=%g",
				meshIndex++,
				r.lods.size(),
				r.biasA,
				r.biasB,
				lod0.edgeCount,
				lod0.maxEdge,
				lod0.avgEdge,
				lod0.minEdge,
				lod0.medianEdge,
				r.mesh->numVerts,
				triCount,
				r.mesh->bounds.radius);
			printf("\n      biasA*edges=%g biasB*edges=%g biasA*edges*avg=%g biasB*edges*avg=%g",
				biasA_edge,
				biasB_edge,
				biasA_edge_avg,
				biasB_edge_avg);
			printf("\n      calcEdges=%u calcMax=%g calcAvg=%g calcMin=%g calcMed=%g",
				calcEdges,
				calcMax,
				calcAvg,
				calcMin,
				calcMed);

			if (r.lods.size() > 1)
			{
				const LodEntry lod1 = r.lods[1];
				const double edgeRatio = (lod0.edgeCount > 0) ? (double(lod1.edgeCount) / double(lod0.edgeCount)) : 0.0;
				const double avgRatio = (lod0.avgEdge > 0.0f) ? (double(lod1.avgEdge) / double(lod0.avgEdge)) : 0.0;
				printf("\n      lod1: edges=%u max=%g avg=%g min=%g med=%g edgeRatio=%g avgRatio=%g",
					lod1.edgeCount,
					lod1.maxEdge,
					lod1.avgEdge,
					lod1.minEdge,
					lod1.medianEdge,
					edgeRatio,
					avgRatio);
			}

			if (csv.is_open())
			{
				const LodEntry lod1 = (r.lods.size() > 1) ? r.lods[1] : LodEntry{};
				csv << meshIndex - 1 << ","
					<< r.biasA << ","
					<< r.biasB << ","
					<< r.mesh->numVerts << ","
					<< triCount << ","
					<< r.mesh->bounds.radius << ","
					<< lod0.edgeCount << ","
					<< lod0.maxEdge << ","
					<< lod0.avgEdge << ","
					<< lod0.minEdge << ","
					<< lod0.medianEdge << ","
					<< lod1.edgeCount << ","
					<< lod1.maxEdge << ","
					<< lod1.avgEdge << ","
					<< lod1.minEdge << ","
					<< lod1.medianEdge
					<< "\n";
			}
		}

		if (csv.is_open())
		{
			csv.close();
			printf("\n[LodBiasDebug] Wrote %s", csvPath);
		}

		std::vector<const FileLodStats*> validRecords;
		for (const auto& r : s_records)
		{
			if (!r.mesh) continue;
			if (r.lods.empty()) continue;
			if (r.lods[0].edgeCount == 0) continue;
			if (r.mesh->triangles.empty()) continue;
			if (r.mesh->numVerts <= 0) continue;
			if (r.mesh->numVerts > 10000000) continue;
			validRecords.push_back(&r);
		}

		std::vector<Candidate> candidates;

		auto add_candidate = [&](const std::string& name, auto getter)
		{
			Candidate c{};
			c.name = name;
			double sumA = 0.0;
			double sumB = 0.0;
			int count = 0;

			for (const auto* rp : validRecords)
			{
				const auto& r = *rp;

				uint32_t calcEdges = 0;
				float calcMax = 0, calcAvg = 0, calcMin = 0, calcMed = 0;
				compute_edge_stats(r.mesh, calcEdges, calcMax, calcAvg, calcMin, calcMed);

				const double v = getter(r, calcEdges, calcMax, calcAvg, calcMin, calcMed);
				sumA += rel_err(r.biasA, v);
				sumB += rel_err(r.biasB, v);
				count++;
			}

			if (count > 0) {
				c.meanRelErrA = sumA / count;
				c.meanRelErrB = sumB / count;
				candidates.push_back(c);
			}
		};

		auto add_candidate_scaled = [&](const std::string& name, auto getter)
		{
			Candidate c{};
			c.name = name;

			double sumF2 = 0.0;
			double sumFA = 0.0;
			double sumFB = 0.0;
			for (const auto* rp : validRecords)
			{
				const auto& r = *rp;
				uint32_t calcEdges = 0;
				float calcMax = 0, calcAvg = 0, calcMin = 0, calcMed = 0;
				compute_edge_stats(r.mesh, calcEdges, calcMax, calcAvg, calcMin, calcMed);
				const double f = getter(r, calcEdges, calcMax, calcAvg, calcMin, calcMed);
				if (f == 0.0) continue;
				sumF2 += f * f;
				sumFA += f * r.biasA;
				sumFB += f * r.biasB;
			}

			if (sumF2 <= 0.0) return;
			c.scaleA = sumFA / sumF2;
			c.scaleB = sumFB / sumF2;

			double sumA = 0.0;
			double sumB = 0.0;
			int count = 0;
			for (const auto* rp : validRecords)
			{
				const auto& r = *rp;
				uint32_t calcEdges = 0;
				float calcMax = 0, calcAvg = 0, calcMin = 0, calcMed = 0;
				compute_edge_stats(r.mesh, calcEdges, calcMax, calcAvg, calcMin, calcMed);
				const double f = getter(r, calcEdges, calcMax, calcAvg, calcMin, calcMed);
				if (f == 0.0) continue;
				sumA += rel_err(r.biasA, c.scaleA * f);
				sumB += rel_err(r.biasB, c.scaleB * f);
				count++;
			}

			if (count > 0) {
				c.meanRelErrA = sumA / count;
				c.meanRelErrB = sumB / count;
				candidates.push_back(c);
			}
		};

		add_candidate("1/edgeCount", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].edgeCount > 0) ? (1.0 / r.lods[0].edgeCount) : 0.0;
		});
		add_candidate_scaled("k/edgeCount", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].edgeCount > 0) ? (1.0 / r.lods[0].edgeCount) : 0.0;
		});
		add_candidate("1/numVerts", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (r.mesh && r.mesh->numVerts > 0) ? (1.0 / r.mesh->numVerts) : 0.0;
		});
		add_candidate_scaled("k/numVerts", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (r.mesh && r.mesh->numVerts > 0) ? (1.0 / r.mesh->numVerts) : 0.0;
		});
		add_candidate("1/numTris", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (r.mesh && !r.mesh->triangles.empty()) ? (1.0 / r.mesh->triangles.size()) : 0.0;
		});
		add_candidate_scaled("k/numTris", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (r.mesh && !r.mesh->triangles.empty()) ? (1.0 / r.mesh->triangles.size()) : 0.0;
		});
		add_candidate("1/maxEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].maxEdge > 0.0f) ? (1.0 / r.lods[0].maxEdge) : 0.0;
		});
		add_candidate_scaled("k/maxEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].maxEdge > 0.0f) ? (1.0 / r.lods[0].maxEdge) : 0.0;
		});
		add_candidate("1/avgEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].avgEdge > 0.0f) ? (1.0 / r.lods[0].avgEdge) : 0.0;
		});
		add_candidate_scaled("k/avgEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].avgEdge > 0.0f) ? (1.0 / r.lods[0].avgEdge) : 0.0;
		});
		add_candidate("1/minEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].minEdge > 0.0f) ? (1.0 / r.lods[0].minEdge) : 0.0;
		});
		add_candidate_scaled("k/minEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].minEdge > 0.0f) ? (1.0 / r.lods[0].minEdge) : 0.0;
		});
		add_candidate("1/medianEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].medianEdge > 0.0f) ? (1.0 / r.lods[0].medianEdge) : 0.0;
		});
		add_candidate_scaled("k/medianEdge", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].medianEdge > 0.0f) ? (1.0 / r.lods[0].medianEdge) : 0.0;
		});
		add_candidate("1/(edgeCount*maxEdge)", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].edgeCount > 0 && r.lods[0].maxEdge > 0.0f) ? (1.0 / (r.lods[0].edgeCount * r.lods[0].maxEdge)) : 0.0;
		});
		add_candidate_scaled("k/(edgeCount*maxEdge)", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].edgeCount > 0 && r.lods[0].maxEdge > 0.0f) ? (1.0 / (r.lods[0].edgeCount * r.lods[0].maxEdge)) : 0.0;
		});
		add_candidate("1/(edgeCount*avgEdge)", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].edgeCount > 0 && r.lods[0].avgEdge > 0.0f) ? (1.0 / (r.lods[0].edgeCount * r.lods[0].avgEdge)) : 0.0;
		});
		add_candidate_scaled("k/(edgeCount*avgEdge)", [](const FileLodStats& r, uint32_t, float, float, float, float){
			return (!r.lods.empty() && r.lods[0].edgeCount > 0 && r.lods[0].avgEdge > 0.0f) ? (1.0 / (r.lods[0].edgeCount * r.lods[0].avgEdge)) : 0.0;
		});

		std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b){
			return (a.meanRelErrA + a.meanRelErrB) < (b.meanRelErrA + b.meanRelErrB);
		});

		const int show = std::min(5, static_cast<int>(candidates.size()));
		printf("\n[LodBiasDebug] Best candidates (lower mean rel error is better):");
		for (int i = 0; i < show; ++i)
		{
			const auto& c = candidates[i];
			if (c.scaleA != 0.0 || c.scaleB != 0.0)
			{
				printf("\n  %s  errA=%g  errB=%g  kA=%g  kB=%g",
					c.name.c_str(),
					c.meanRelErrA,
					c.meanRelErrB,
					c.scaleA,
					c.scaleB);
			}
			else
			{
				printf("\n  %s  errA=%g  errB=%g", c.name.c_str(), c.meanRelErrA, c.meanRelErrB);
			}
		}
		printf("\n");
	}
}
