//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    // Directional light
    struct directional_light_s
    {
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;
        DirectX::XMFLOAT3 direction;
        float pad = 0.0f;
    };

    using DirectionalLight = directional_light_s;

    struct point_light_s
    {
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;

        // (position, range)
        DirectX::XMFLOAT3 position;
        float range;

        // (A0, A1, A2, pad)
        DirectX::XMFLOAT3 att;
        float pad = 0.0f;
    };

    using PointLight = point_light_s;

    struct material_s
    {
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular; // w = Specular Power
        DirectX::XMFLOAT4 reflect;
    };

    using Material = material_s;
}































