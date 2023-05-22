//
// Created by ZZK on 2023/5/22.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    struct vertex_pos_tex_s
    {
        vertex_pos_tex_s() = default;

        vertex_pos_tex_s(const vertex_pos_tex_s&) = default;
        vertex_pos_tex_s& operator=(const vertex_pos_tex_s&) = default;

        vertex_pos_tex_s(vertex_pos_tex_s&&) = default;
        vertex_pos_tex_s& operator=(vertex_pos_tex_s&&) = default;

        constexpr vertex_pos_tex_s(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT2& _tex)
        : pos(_pos), tex(_tex)
        {

        }

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 2>& get_input_layout();

        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 tex;
    };

    using VertexPosTex = vertex_pos_tex_s;

    struct vertex_pos_color_s
    {
        vertex_pos_color_s() = default;

        vertex_pos_color_s(const vertex_pos_color_s&) = default;
        vertex_pos_color_s& operator=(const vertex_pos_color_s&) = default;

        vertex_pos_color_s(vertex_pos_color_s&&) = default;
        vertex_pos_color_s& operator=(vertex_pos_color_s&&) = default;

        constexpr vertex_pos_color_s(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT4& _color)
        : pos(_pos), color(_color)
        {

        }

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 2>& get_input_layout();

        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

    using VertexPosColor = vertex_pos_color_s;

    struct vertex_pos_normal_tex_s
    {
        vertex_pos_normal_tex_s() = default;

        vertex_pos_normal_tex_s(const vertex_pos_normal_tex_s&) = default;
        vertex_pos_normal_tex_s& operator=(const vertex_pos_normal_tex_s&) = default;

        vertex_pos_normal_tex_s(vertex_pos_normal_tex_s&&) = default;
        vertex_pos_normal_tex_s& operator=(vertex_pos_normal_tex_s&&) = default;

        constexpr vertex_pos_normal_tex_s(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT3& _normal,
                                            const DirectX::XMFLOAT2& _tex)
        : pos(_pos), normal(_normal), tex(_tex)
        {

        }

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 3>& get_input_layout();

        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 tex;
    };

    using VertexPosNormalTex = vertex_pos_normal_tex_s;

    struct vertex_pos_normal_tangent_tex_s
    {
        vertex_pos_normal_tangent_tex_s() = default;

        vertex_pos_normal_tangent_tex_s(const vertex_pos_normal_tangent_tex_s&) = default;
        vertex_pos_normal_tangent_tex_s& operator=(const vertex_pos_normal_tangent_tex_s&) = default;

        vertex_pos_normal_tangent_tex_s(vertex_pos_normal_tangent_tex_s&&) = default;
        vertex_pos_normal_tangent_tex_s& operator=(vertex_pos_normal_tangent_tex_s&&) = default;

        constexpr vertex_pos_normal_tangent_tex_s(const DirectX::XMFLOAT3& _pos, const DirectX::XMFLOAT3& _normal,
                                            const DirectX::XMFLOAT4& _tangent, const DirectX::XMFLOAT2& _tex)
        : pos(_pos), normal(_normal), tangent(_tangent), tex(_tex)
        {

        }

        static const std::array<D3D11_INPUT_ELEMENT_DESC, 4>& get_input_layout();

        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 tangent;
        DirectX::XMFLOAT2 tex;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[4];
    };

    using VertexPosNormalTangentTex = vertex_pos_normal_tangent_tex_s;
}
