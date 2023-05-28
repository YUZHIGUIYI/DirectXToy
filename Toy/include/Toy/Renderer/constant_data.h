//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Renderer/illuminant.h>

namespace toy
{
    constexpr uint32_t max_lights = 2;
    // Data structure corresponding to constant buffer
    // See HLSL structure, aligned by 16 bytes
    struct CBChangesEveryDraw
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX world_inv_transpose;
        // TODO: include material
        Material material;
    };

    struct CBDrawStates
    {
        int32_t is_reflection = 0;
        int32_t is_shadow = 0;
        DirectX::XMINT2 pad;
    };

    struct CBChangesEveryFrame
    {
        DirectX::XMMATRIX view;
        DirectX::XMFLOAT3 eye_pos;
        float pad;
    };

    struct CBChangesOnResize
    {
        DirectX::XMMATRIX proj;
    };

    struct CBChangesRarely
    {
        DirectX::XMMATRIX reflection;
        DirectX::XMMATRIX shadow;
        DirectX::XMMATRIX ref_shadow;
        DirectionalLight dir_light[max_lights];
        PointLight point_light[max_lights];
    };
}
