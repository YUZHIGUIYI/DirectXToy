//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Renderer/shader_defines.h>

namespace toy
{
    static constexpr uint32_t max_lights = 5;
    // Directional light
    struct DirectionalLight
    {
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;
        DirectX::XMFLOAT3 direction;
        float pad = 0.0f;
    };

    struct PointLight
    {
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;

        // (position, range)
        DirectX::XMFLOAT3 position;
        float range = 1.0f;

        // (A0, A1, A2, pad)
        DirectX::XMFLOAT3 att;
        float pad = 0.0f;
    };

    struct PhongMaterial
    {
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular; // w = Specular Power
        DirectX::XMFLOAT4 reflect;
    };

    // Corresponds to "PointLight" in shader
    struct PointLightApp
    {
        DirectX::XMFLOAT3 pos_v;
        float attenuation_begin;
        DirectX::XMFLOAT3 color;
        float attenuation_end;
    };

    // Initial transformation data - cylindrical coordinate system
    struct PointLightInitData
    {
        float radius;
        float angle;
        float height;
        float animation_speed;
    };

    struct TileInfo
    {
        uint32_t num_lights;
        uint32_t light_indices[MAX_LIGHT_INDICES];
    };
}































