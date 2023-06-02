//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Renderer/effect_helper.h>
#include <Toy/Renderer/constant_data.h>
#include <Toy/Renderer/effect_interface.h>

namespace toy
{
    class BasicEffect : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        BasicEffect();
        ~BasicEffect() override = default;

        BasicEffect(BasicEffect&& other) noexcept;
        BasicEffect& operator=(BasicEffect&& other) noexcept;

        // Init all resources
        void init(ID3D11Device* device);

        // Transform
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

        // Material
        void set_material(const model::Material& material) override;

        // Mesh data
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // Draw by default
        void set_default_render();

        // Light
        void set_dir_light(uint32_t pos, const DirectionalLight& dir_light);
        void set_point_light(uint32_t pos, const PointLight& point_light);

        void set_eye_pos(const DirectX::XMFLOAT3& eye_pos);

        // Update and bind constant buffers
        void apply(ID3D11DeviceContext * device_context) override;

    private:
        struct effect_impl
        {
            effect_impl() = default;
            ~effect_impl() = default;

            std::unique_ptr<EffectHelper> m_effect_helper;
            std::shared_ptr<IEffectPass> m_curr_effect_pass;
            com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;
            com_ptr<ID3D11InputLayout> m_curr_input_layout;

            D3D11_PRIMITIVE_TOPOLOGY m_curr_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            DirectX::XMFLOAT4X4 m_world{};
            DirectX::XMFLOAT4X4 m_view{};
            DirectX::XMFLOAT4X4 m_proj{};
        };

        std::unique_ptr<effect_impl> m_effect_impl;
    };
}







































