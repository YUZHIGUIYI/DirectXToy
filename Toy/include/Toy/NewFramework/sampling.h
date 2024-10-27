//
// Created by ZZK on 2024/10/26.
//

#pragma once

#include <cySampleElim.h>
#include <cyVector.h>
#include <DirectXMath.h>
#include <random>

namespace toy
{
	inline std::vector<DirectX::XMFLOAT2> fast_poisson_disk_sampling(uint32_t sample_count)
	{
		static_assert(sizeof(DirectX::XMFLOAT2) == sizeof(cy::Vec2f), "Vector size should be equal to DirectX vector size");
		std::default_random_engine rng{ std::random_device{}() };
		std::uniform_real_distribution distribution{ 0.0f, 1.0f };

		std::vector<cy::Vec2f> raw_points{};
		for (uint32_t i = 0; i < sample_count * 10; ++i)
		{
			auto u = distribution(rng);
			auto v = distribution(rng);
			raw_points.emplace_back(u, v);
		}
		std::vector<cy::Vec2f> output_points(sample_count);
		cy::WeightedSampleElimination<cy::Vec2f, float, 2> weighted_sample_elimination{};
		weighted_sample_elimination.SetTiling(true);
		weighted_sample_elimination.Eliminate(raw_points.data(), raw_points.size(),
											output_points.data(), output_points.size());
		std::vector<DirectX::XMFLOAT2> result(sample_count);
		std::memcpy(result.data(), output_points.data(), sizeof(cy::Vec2f) * sample_count);
		return result;
	}
}





