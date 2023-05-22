//
// Created by ZZK on 2023/5/22.
//

#include <Toy/Geometry/vertex.h>

namespace toy
{
    const std::array<D3D11_INPUT_ELEMENT_DESC, 2>& vertex_pos_tex_s::get_input_layout()
    {
        static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> input_layout{
            D3D11_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            D3D11_INPUT_ELEMENT_DESC{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        return input_layout;
    }

    const std::array<D3D11_INPUT_ELEMENT_DESC, 2>& vertex_pos_color_s::get_input_layout()
    {
        static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> input_layout{
            D3D11_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            D3D11_INPUT_ELEMENT_DESC{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        return input_layout;
    }

    const std::array<D3D11_INPUT_ELEMENT_DESC, 3>& vertex_pos_normal_tex_s::get_input_layout()
    {
        static const std::array<D3D11_INPUT_ELEMENT_DESC, 3> input_layout{
            D3D11_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            D3D11_INPUT_ELEMENT_DESC{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            D3D11_INPUT_ELEMENT_DESC{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        return input_layout;
    }

    const std::array<D3D11_INPUT_ELEMENT_DESC, 4>& vertex_pos_normal_tangent_tex_s::get_input_layout()
    {
        static const std::array<D3D11_INPUT_ELEMENT_DESC, 4> input_layout{
            D3D11_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            D3D11_INPUT_ELEMENT_DESC{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            D3D11_INPUT_ELEMENT_DESC{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            D3D11_INPUT_ELEMENT_DESC{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        return input_layout;
    }
}



















