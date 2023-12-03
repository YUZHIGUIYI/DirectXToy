//
// Created by ZZK on 2023/5/22.
//

#pragma once

#include <Toy/Geometry/vertex.h>

namespace toy::geometry
{
    struct GeometryData
    {
        std::vector<DirectX::XMFLOAT3> vertices;
        std::vector<DirectX::XMFLOAT3> normals;
        std::vector<DirectX::XMFLOAT2> texcoords;
        std::vector<DirectX::XMFLOAT4> tangents;
        std::vector<uint32_t> indices32;
        std::vector<uint16_t> indices16;
        std::vector<uint32_t> entity_id_array;
    };

    // Create sphere mesh data, the bigger the slices and the levels, the higher the accuracy
    GeometryData create_sphere(uint32_t entity_id = 1, float radius = 1.0f, uint32_t levels = 20, uint32_t slices = 20);

    // Create box mesh data
    GeometryData create_box(uint32_t entity_id = 1, float width = 2.0f, float height = 2.0f, float depth = 2.0f);

    // Create cylinder mesh data, the bigger the slices, the higher the accuracy
    GeometryData create_cylinder(uint32_t entity_id = 1, float radius = 1.0f, float height = 2.0f, uint32_t slices = 20, uint32_t stacks = 10, float texU = 1.0f, float texV = 1.0f);

    // Create cone mesh data, the bigger slices, the higher the accuracy
    GeometryData create_cone(uint32_t entity_id = 1, float radius = 1.0f, float height = 2.0f, uint32_t slices = 20);

    // Create a plane
    GeometryData create_plane(uint32_t entity_id, const DirectX::XMFLOAT2& planeSize, const DirectX::XMFLOAT2& maxTexCoord = { 1.0f, 1.0f });
    GeometryData create_plane(uint32_t entity_id = 1, float width = 10.0f, float depth = 10.0f, float texU = 1.0f, float texV = 1.0f);

    // Create a grid
    GeometryData create_grid(uint32_t entity_id, const DirectX::XMFLOAT2& gridSize, const DirectX::XMUINT2& slices, const DirectX::XMFLOAT2& maxTexCoord,
                            const std::function<float(float, float)>& heightFunc = [](float x, float z) { return 0.0f; },
                            const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc = [](float x, float z) { return DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f); },
                            const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc = [](float x, float z) { return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); });
}
















