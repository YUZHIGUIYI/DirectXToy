//
// Created by ZZK on 2024/3/22.
//

#include <Sandbox/Runtime/picking_system.h>
#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/events.h>
#include <Toy/Runtime/renderer.h>
#include <Toy/Runtime/input_controller.h>
#include <Sandbox/Runtime/editing_system.h>

namespace toy::editor
{
    PickingSystem::PickingSystem()
    {
        runtime::on_frame_update = [this] (float delta_time)
        {
            this->on_update(delta_time);
        };
    }

    void PickingSystem::reset_viewport_setting(const ViewportSetting &viewport_setting)
    {
        m_viewport_setting = viewport_setting;
    }

    void PickingSystem::on_update(float delta_time)
    {
        auto&& editing_system = core::get_subsystem<EditingSystem>();
        auto&& renderer = core::get_subsystem<runtime::Renderer>();
        auto&& input_controller = core::get_subsystem<runtime::InputController>();

        if (ImGuizmo::IsUsing() || !input_controller.is_mouse_button_pressed(mouse::ButtonLeft)) return;

        auto [mouse_pos_x, mouse_pos_y] = ImGui::GetMousePos();
        mouse_pos_x -= m_viewport_setting.lower_bound.x;
        mouse_pos_y -= m_viewport_setting.lower_bound.y;
        auto relative_mouse_pos_x  = static_cast<int32_t>(mouse_pos_x);
        auto relative_mouse_pos_y = static_cast<int32_t>(mouse_pos_y);
        if (relative_mouse_pos_x > m_viewport_setting.width || relative_mouse_pos_y > m_viewport_setting.height) return;

        auto entity_id_texture = renderer.get_gbuffer_definition().entity_id_buffer->get_texture();
        D3D11_TEXTURE2D_DESC staging_desc = {};
        entity_id_texture->GetDesc(&staging_desc);
        if (staging_desc.Width != m_staging_width || staging_desc.Height != m_staging_height)
        {
            m_staging_texture.Reset();
            m_staging_width = staging_desc.Width;
            m_staging_height = staging_desc.Height;
            staging_desc.Usage = D3D11_USAGE_STAGING;
            staging_desc.BindFlags = 0;
            staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

            renderer.get_device()->CreateTexture2D(&staging_desc, nullptr, m_staging_texture.GetAddressOf());
        }

        renderer.get_device_context()->CopyResource(m_staging_texture.Get(), entity_id_texture);

        auto entity_id = get_entity_id(renderer.get_device_context(), relative_mouse_pos_x, relative_mouse_pos_y);
        DX_INFO("Mouse pos: {} + {}; entity id: {}", mouse_pos_x, mouse_pos_y, entity_id);

        if (entity_id == 0 || entity_id == 1)
        {
            // Note: entity id 0 represents editor camera, 1 represents skybox
            editing_system.unselect();
        } else
        {
            editing_system.select(entity_id);
        }
    }

    uint32_t PickingSystem::get_entity_id(ID3D11DeviceContext *device_context, int32_t mouse_pos_x, int32_t mouse_pos_y)
    {
        static std::vector<uint32_t> staging_data;

        if (mouse_pos_x >= m_staging_width || mouse_pos_y >= m_staging_height) return 0;

        staging_data.clear();
        staging_data.resize(m_staging_width * m_staging_height, 0);

        D3D11_MAPPED_SUBRESOURCE mapped_subresource = {};
        device_context->Map(m_staging_texture.Get(), 0, D3D11_MAP_READ, 0, &mapped_subresource);
        auto texture_data = reinterpret_cast<uint8_t *>(mapped_subresource.pData);
        auto reinterpret_staging_data = reinterpret_cast<uint8_t *>(staging_data.data());
        auto stride = mapped_subresource.RowPitch;
        auto multiple = sizeof(std::decay_t<decltype(staging_data)>::value_type) / sizeof(uint8_t);
        for (uint32_t i = 0; i < m_staging_height; ++i)
        {
            memcpy_s(reinterpret_staging_data + i * m_staging_width * multiple, m_staging_width * multiple, texture_data, m_staging_width * multiple);
            texture_data += stride;
        }
        device_context->Unmap(m_staging_texture.Get(), 0);

        auto index = static_cast<size_t>(mouse_pos_y * m_staging_width + mouse_pos_x - 1);
        return staging_data[index];
    }
}