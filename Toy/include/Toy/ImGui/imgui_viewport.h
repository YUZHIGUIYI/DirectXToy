//
// Created by ZHIKANG on 2023/6/24.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class Viewport
    {
    public:
        static void set_viewport(std::string_view viewport_name, ID3D11ShaderResourceView* srv, int32_t &width, int32_t &height);
    };
}
