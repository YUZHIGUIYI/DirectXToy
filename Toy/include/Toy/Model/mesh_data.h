//
// Created by ZZK on 2023/5/31.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy::model
{
    struct MeshData
    {
        com_ptr<ID3D11Buffer> vertices;
        com_ptr<ID3D11Buffer> normals;
        std::vector<com_ptr<ID3D11Buffer>> texcoord_arrays;
        com_ptr<ID3D11Buffer> tangents;
        com_ptr<ID3D11Buffer> bitangents;
        com_ptr<ID3D11Buffer> colors;

        com_ptr<ID3D11Buffer> indices;

        uint32_t vertex_count = 0;
        uint32_t index_count = 0;
        uint32_t material_index = 0;

        DirectX::BoundingBox bounding_box;
        bool in_frustum = true;
    };
}






















