//
// Created by ZZK on 2024/1/7.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Renderer/buffer.h>

namespace toy
{
    // Standard wire cube indices
    static constexpr std::array<uint16_t, 24> s_wire_cube_indices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    struct GizmosWireEffect::EffectImpl
    {
        com_ptr<ID3D11InputLayout> vertex_pos_layout = nullptr;
        com_ptr<ID3D11Buffer> vertex_buffer = nullptr;
        com_ptr<ID3D11Buffer> index_buffer = nullptr;

        std::unique_ptr<EffectHelper> effect_helper = nullptr;
        std::shared_ptr<IEffectPass> cur_effect_pass = nullptr;

        DirectX::XMFLOAT4 wire_color = { 0.0f, 1.0f, 0.0f, 1.0f };
        DirectX::XMFLOAT4X4 world_matrix = {};
        DirectX::XMFLOAT4X4 view_matrix = {};
        DirectX::XMFLOAT4X4 proj_matrix = {};

        std::string_view gizmos_wire_pass = {};
    };

    GizmosWireEffect::GizmosWireEffect()
    : m_effect_impl(std::make_unique<EffectImpl>())
    {

    }

    GizmosWireEffect::~GizmosWireEffect() = default;

    GizmosWireEffect::GizmosWireEffect(GizmosWireEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    GizmosWireEffect& GizmosWireEffect::operator=(GizmosWireEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    GizmosWireEffect& GizmosWireEffect::get()
    {
        static GizmosWireEffect gizmos_wire_effect = {};
        return gizmos_wire_effect;
    }

    void GizmosWireEffect::init(ID3D11Device *device)
    {
        // Create vertex buffer and index buffer
        {
            CD3D11_BUFFER_DESC buffer_desc = {};
            buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
            buffer_desc.ByteWidth = static_cast<uint32_t>(sizeof(DirectX::XMFLOAT3) * 8);
            buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            device->CreateBuffer(&buffer_desc, nullptr, m_effect_impl->vertex_buffer.GetAddressOf());

            buffer_desc.Usage = D3D11_USAGE_DEFAULT;
            buffer_desc.ByteWidth = static_cast<uint32_t>(sizeof(uint16_t) * s_wire_cube_indices.size());
            buffer_desc.CPUAccessFlags = 0;
            buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA init_data{ s_wire_cube_indices.data(), 0, 0 };
            device->CreateBuffer(&buffer_desc, &init_data, m_effect_impl->index_buffer.GetAddressOf());
        }


        // Set effect cache
        m_effect_impl->effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/pbr/cache");

        // Set shader name and pass name
        std::string_view gizmos_wire_vs = "GizmosWireVS";
        std::string_view gizmos_wire_ps = "GizmosWirePS";
        m_effect_impl->gizmos_wire_pass = "GizmosWirePass";

        // Create vertex and pixel shaders and input layout
        com_ptr<ID3DBlob> blob = nullptr;
        m_effect_impl->effect_helper->create_shader_from_file(gizmos_wire_vs, DXTOY_HOME L"data/pbr/gizmos_wire_vs.hlsl", device,
                                                                "VS", "vs_5_0", nullptr, blob.GetAddressOf());
        m_effect_impl->effect_helper->create_shader_from_file(gizmos_wire_ps, DXTOY_HOME L"data/pbr/gizmos_wire_ps.hlsl", device,
                                                                "PS", "ps_5_0");

        auto&& input_layout = VertexPos::get_input_layout();
        device->CreateInputLayout(input_layout.data(), (uint32_t)input_layout.size(),
                                    blob->GetBufferPointer(), blob->GetBufferSize(),
                                    m_effect_impl->vertex_pos_layout.ReleaseAndGetAddressOf());
        if (!m_effect_impl->vertex_pos_layout)
        {
            DX_CORE_CRITICAL("Failed to create vertex layout");
        }

        // Create geometry and deferred lighting passes
        EffectPassDesc pass_desc = {};
        pass_desc.nameVS = gizmos_wire_vs;
        pass_desc.namePS = gizmos_wire_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->gizmos_wire_pass, device, &pass_desc);

        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->gizmos_wire_pass);
        if (m_effect_impl->cur_effect_pass)
        {
            m_effect_impl->cur_effect_pass->set_rasterizer_state(RenderStates::rs_no_cull.Get());
            m_effect_impl->cur_effect_pass->set_depth_stencil_state(RenderStates::dss_greater_equal.Get(), 0);
        } else
        {
            DX_CORE_CRITICAL("Failed to create gizmos wire effect pass");
        }
    }

    void GizmosWireEffect::set_wire_color(const DirectX::XMFLOAT3 &wire_color)
    {
        m_effect_impl->wire_color = DirectX::XMFLOAT4{ wire_color.x, wire_color.y, wire_color.z, 1.0f };
    }

    void GizmosWireEffect::set_vertex_buffer(ID3D11DeviceContext *device_context, const DirectX::BoundingBox &bounding_box)
    {
        using namespace DirectX;

        std::vector<XMFLOAT3> temp_wire_cube_data;
        temp_wire_cube_data.reserve(8);

        auto&& bounding_center = bounding_box.Center;
        auto half_width = bounding_box.Extents.x;
        auto half_height = bounding_box.Extents.y;
        auto half_depth = bounding_box.Extents.z;

        temp_wire_cube_data.emplace_back(bounding_center.x + half_width, bounding_center.y - half_height, bounding_center.z - half_depth);
        temp_wire_cube_data.emplace_back(bounding_center.x + half_width, bounding_center.y - half_height, bounding_center.z + half_depth);
        temp_wire_cube_data.emplace_back(bounding_center.x + half_width, bounding_center.y + half_height, bounding_center.z + half_depth);
        temp_wire_cube_data.emplace_back(bounding_center.x + half_width, bounding_center.y + half_height, bounding_center.z - half_depth);
        temp_wire_cube_data.emplace_back(bounding_center.x - half_width, bounding_center.y - half_height, bounding_center.z - half_depth);
        temp_wire_cube_data.emplace_back(bounding_center.x - half_width, bounding_center.y - half_height, bounding_center.z + half_depth);
        temp_wire_cube_data.emplace_back(bounding_center.x - half_width, bounding_center.y + half_height, bounding_center.z + half_depth);
        temp_wire_cube_data.emplace_back(bounding_center.x - half_width, bounding_center.y + half_height, bounding_center.z - half_depth);

        D3D11_MAPPED_SUBRESOURCE mapped_resource = {};
        device_context->Map(m_effect_impl->vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
        std::memcpy(mapped_resource.pData, temp_wire_cube_data.data(), sizeof(XMFLOAT3) * temp_wire_cube_data.size());
        device_context->Unmap(m_effect_impl->vertex_buffer.Get(), 0);
    }

    void GizmosWireEffect::render(ID3D11DeviceContext *device_context, ID3D11RenderTargetView *out_rtv,
                                    ID3D11DepthStencilView *input_dsv, const D3D11_VIEWPORT &viewport)
    {
        using namespace DirectX;

        constexpr uint32_t vertex_stride = sizeof(XMFLOAT3);
        constexpr uint32_t vertex_offset = 0;

        XMVECTOR wire_color = XMLoadFloat4(&m_effect_impl->wire_color);

        XMMATRIX world_matrix = XMLoadFloat4x4(&m_effect_impl->world_matrix);
        XMMATRIX view_matrix = XMLoadFloat4x4(&m_effect_impl->view_matrix);
        XMMATRIX proj_matrix = XMLoadFloat4x4(&m_effect_impl->proj_matrix);
        XMMATRIX world_view_proj_matrix = world_matrix * view_matrix * proj_matrix;
        world_view_proj_matrix = XMMatrixTranspose(world_view_proj_matrix);

        // Apply pass and draw lines
        m_effect_impl->effect_helper->get_constant_buffer_variable("gWorldViewProj")->set_float_matrix(4, 4, (float *)&world_view_proj_matrix);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gWireColor")->set_float_vector(4, (float *)&wire_color);
        device_context->IASetInputLayout(m_effect_impl->vertex_pos_layout.Get());
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        device_context->IASetVertexBuffers(0, 1, m_effect_impl->vertex_buffer.GetAddressOf(), &vertex_stride, &vertex_offset);
        device_context->IASetIndexBuffer(m_effect_impl->index_buffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        device_context->RSSetViewports(1, &viewport);
        device_context->OMSetRenderTargets(1, &out_rtv, input_dsv);
        m_effect_impl->cur_effect_pass->apply(device_context);
        device_context->DrawIndexed(static_cast<uint32_t>(s_wire_cube_indices.size()), 0, 0);

        // Clear binding
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void XM_CALLCONV GizmosWireEffect::set_world_matrix(DirectX::FXMMATRIX &world)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->world_matrix, world);
    }

    void XM_CALLCONV GizmosWireEffect::set_view_matrix(DirectX::FXMMATRIX &view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->view_matrix, view);
    }

    void XM_CALLCONV GizmosWireEffect::set_proj_matrix(DirectX::FXMMATRIX &proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->proj_matrix, proj);
    }
}