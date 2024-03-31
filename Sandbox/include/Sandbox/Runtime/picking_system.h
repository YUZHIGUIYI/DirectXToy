//
// Created by ZZK on 2024/3/22.
//

#pragma once

#include <Sandbox/Core/common.h>

namespace toy::editor
{
    struct PickingSystem
    {
    public:
        PickingSystem();

        void reset_viewport_setting(const ViewportSetting &viewport_setting);

        void on_update(float delta_time);

    private:
        uint32_t get_entity_id(ID3D11DeviceContext *device_context, int32_t mouse_pos_x, int32_t mouse_pos_y);

    private:
        com_ptr<ID3D11Texture2D> m_staging_texture = nullptr;
        ViewportSetting m_viewport_setting = {};
        uint32_t m_staging_width = 0;
        uint32_t m_staging_height = 0;
    };
}
