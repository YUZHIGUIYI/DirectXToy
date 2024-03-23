//
// Created by ZZK on 2024/3/23.
//

#pragma once

#include <Toy/toy.h>

namespace toy::editor
{
    struct ViewportSetting
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

    struct GizmoSnap
    {
        std::array<float, 3> translation_snap = { 1.0f, 1.0f, 1.0f };
        float rotation_degree_snap = 15.0f;
        float scale_snap = 0.1f;
    };
}
