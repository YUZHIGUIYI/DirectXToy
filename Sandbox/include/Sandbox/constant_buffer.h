//
// Created by ZZK on 2023/5/24.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    // TODO: include material
    struct cb_change_every_draw_s
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX world_inv_transpose;
    };
    using CBChangeEveryDraw = cb_change_every_draw_s;

    struct cb_draw_states_s
    {
        int32_t is_reflection;
        DirectX::XMFLOAT3 pad;
    };
    using CBDrawStates = cb_draw_states_s;

    struct cb_change_every_frame_s
    {
        DirectX::XMMATRIX view;
        DirectX::XMFLOAT4 eye_pos;
    };
    using CBChangeEveryFrame = cb_change_every_frame_s;

    struct cb_change_on_resize_s
    {
        DirectX::XMMATRIX proj;
    };
    using CBChangeOnResize = cb_change_on_resize_s;

    struct cb_change_rarely_s
    {
        DirectX::XMMATRIX reflection;
    };
    using CBChangeRarely = cb_change_rarely_s;
}
