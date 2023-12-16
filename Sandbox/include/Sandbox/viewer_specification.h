//
// Created by ZZK on 2023/12/4.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    struct ViewerSpecification
    {
        int32_t width = 640;
        int32_t height = 360;

        DirectX::XMFLOAT2 lower_bound{};
        DirectX::XMFLOAT2 upper_bound{};

        [[nodiscard]] float get_aspect_ratio() const
        {
            return static_cast<float>(width) / static_cast<float>(height);
        }
    };
}
