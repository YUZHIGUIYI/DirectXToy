//
// Created by ZZK on 2023/10/6.
//

#include <Toy/Core/base.h>

namespace toy::taa
{
    // * 8x TAA - 2
    inline static constexpr std::array<float, 8> s_halton_2{
             0.0f,
            -1.0f / 2.0f,
             1.0f / 2.0f,
            -3.0f / 4.0f,
             1.0f / 4.0f,
            -1.0f / 4.0f,
             3.0f / 4.0f,
            -7.0f / 8.0f
    };
    // * 8x TAA - 3
    inline static constexpr std::array<float, 8> s_halton_3{
            -1.0f / 3.0f,
             1.0f / 3.0f,
            -7.0f / 9.0f,
            -1.0f / 9.0f,
             5.0f / 9.0f,
            -5.0f / 9.0f,
             1.0f / 9.0f,
             7.0f / 9.0f
    };

    // * TAA sample
    inline static constexpr uint32_t s_taa_sample = 8;

    // * TAA jitter distance
    inline static constexpr float s_taa_jitter_distance = 0.5f;
}
