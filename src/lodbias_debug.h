#pragma once
#include <vector>
#include <string>
#include "meshstructs.h"

namespace LodBiasDebug
{
	struct LodEntry
	{
		uint32_t edgeCount = 0;
		float maxEdge = 0.0f;
		float avgEdge = 0.0f;
		float minEdge = 0.0f;
		float medianEdge = 0.0f;
	};

	struct FileLodStats
	{
		const Mesh* mesh = nullptr;
		float biasA = 0.0f;
		float biasB = 0.0f;
		std::vector<LodEntry> lods;
	};

	void record(const Mesh* mesh,
		float biasA,
		float biasB,
		std::vector<LodEntry> lods);

	void report();
}
