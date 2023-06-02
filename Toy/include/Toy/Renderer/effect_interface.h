//
// Created by ZZK on 2023/5/31.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    namespace model
    {
        class Material;
        class MeshData;
    }
    // Single mesh data transfers to input assembly stage
    // Effect pass supports input layout, strides, offsets and primitives
    // Other data comes from mesh data
    struct MeshDataInput
    {
        ID3D11InputLayout* input_layout = nullptr;
        D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        std::vector<ID3D11Buffer*> vertex_buffers;
        ID3D11Buffer* index_buffer = nullptr;
        std::vector<uint32_t> strides;
        std::vector<uint32_t> offsets;
        uint32_t index_count = 0;
    };

    class IEffect
    {
    public:
        IEffect() = default;
        virtual ~IEffect() = default;
        // Disable copy, allow move
        IEffect(const IEffect&) = delete;
        IEffect& operator=(const IEffect&) = delete;
        IEffect(IEffect&&) = default;
        IEffect& operator=(IEffect&&) = default;

        // Update and bind constant buffers
        virtual void apply(ID3D11DeviceContext * device_context) = 0;
    };

    class IEffectTransform
    {
    public:
        virtual void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) = 0;
        virtual void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) = 0;
        virtual void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) = 0;
    };

    class IEffectMaterial
    {
    public:
        virtual void set_material(const model::Material& material) = 0;
    };

    class IEffectMeshData
    {
    public:
        virtual MeshDataInput get_input_data(const model::MeshData& mesh_data) = 0;
    };
}





































