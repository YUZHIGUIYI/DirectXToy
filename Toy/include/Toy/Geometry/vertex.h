//
// Created by ZZK on 2023/5/22.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    struct VertexPos
    {
        DirectX::XMFLOAT3 pos;

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 1> &get_input_layout();
    };

    struct VertexPosTex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 tex;

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> &get_input_layout();
    };

    struct VertexPosColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> &get_input_layout();
    };

    struct VertexPosNormalTex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 tex;

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 3> &get_input_layout();
    };

    struct VertexPosNormalTangentTex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 tangent;
        DirectX::XMFLOAT2 tex;

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 4> &get_input_layout();
    };

    struct VertexPosNormalTangentTexEntity
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 tangent;
        DirectX::XMFLOAT2 tex;
        uint32_t          entity_id;

        static const std::array<D3D11_INPUT_ELEMENT_DESC , 5> &get_input_layout();
    };
}
