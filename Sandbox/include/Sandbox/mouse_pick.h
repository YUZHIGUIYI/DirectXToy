//
// Created by ZZK on 2023/11/30.
//

#include <Toy/toy.h>

namespace toy
{
    struct MousePickHelper
    {
    public:
        std::vector<uint32_t> staging_data = {};
        com_ptr<ID3D11Texture2D> staging_texture = nullptr;
        uint32_t staging_width = 0;
        uint32_t staging_height = 0;

        uint32_t pick_entity(ID3D11Device *device, ID3D11DeviceContext *device_context, ID3D11Texture2D *entity_id_buffer, int32_t mouse_pos_x, int32_t mouse_pos_y);

    private:
        uint32_t get_entity_id(ID3D11DeviceContext *device_context, uint32_t mouse_pos_x, uint32_t mouse_pos_y);
    };
}
