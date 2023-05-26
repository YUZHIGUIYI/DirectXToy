//
// Created by ZZK on 2023/5/22.
//

#pragma once

#include <Toy/Geometry/vertex.h>

namespace toy::geometry
{
    // Mesh data
    template<typename VertexType = VertexPosNormalTex, typename IndexType = uint32_t>
    struct mesh_data_s
    {
        using VT = VertexType;
        using IT = IndexType;

        std::vector<VertexType> vertices;   // Vertex vector
        std::vector<IndexType> indices;     // Index vector

        mesh_data_s()
        {
            static_assert(sizeof(IndexType) == 2 || sizeof(IndexType) == 4, "The size of IndexType must be 2 bytes or 4 bytes");
            static_assert(std::is_unsigned_v<IndexType>, "IndexType must be unsigned integer");
        }
    };

    // Alias
    template<typename VertexType = VertexPosNormalTex, typename IndexType = uint32_t>
    using MeshData = mesh_data_s<VertexType, IndexType>;

    // Create sphere mesh data
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_sphere(float radius = 1.0f, uint32_t levels = 20, uint32_t slices = 30,
                                                    const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    // Create box mesh data
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_box(float width = 2.0f, float height = 2.0f, float depth = 2.0f,
                                                const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    // Create cone mesh data, the bigger the slices, the higher the accuracy
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_cone(float radius = 1.0f, float height = 2.0f, uint32_t slices = 30,
                                                const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    // Create cone mesh data with side faces only, the bigger the slices, the higher the accuracy
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_cone_no_cap(float radius = 1.0f, float height = 2.0f, uint32_t slices = 20,
                                                    const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    // Create a NDC face
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_2d_show(const DirectX::XMFLOAT2& center, const DirectX::XMFLOAT2& scale, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_2d_show(float centerX = 0.0f, float centerY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    // Create a plane
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_plane(const DirectX::XMFLOAT2& planeSize, const DirectX::XMFLOAT2& maxTexCoord = { 1.0f, 1.0f },
                                                    const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
    template<class VertexType = VertexPosNormalTex, class IndexType = uint32_t>
    MeshData<VertexType, IndexType> create_plane(float width = 10.0f, float depth = 10.0f, float texU = 1.0f, float texV = 1.0f,
                                                    const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
}

namespace toy::geometry::internal
{
    struct vertex_data_s
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 tangent;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 tex;
    };

    using VertexData = vertex_data_s;

    // 根据目标顶点类型选择性将数据插入
    template<class VertexType>
    void insert_vertex_element(VertexType& vertex_dst, const VertexData& vertex_src)
    {
        static std::string semantic_name;
        static const std::map<std::string, std::pair<size_t, size_t>> semantic_siz_map{
            {"POSITION", std::pair<size_t, size_t>(0, 12)},
            {"NORMAL", std::pair<size_t, size_t>(12, 24)},
            {"TANGENT", std::pair<size_t, size_t>(24, 40)},
            {"COLOR", std::pair<size_t, size_t>(40, 56)},
            {"TEXCOORD", std::pair<size_t, size_t>(56, 64)}
        };

        auto&& input_layout = VertexType::get_input_layout();
        for (size_t i = 0; i < input_layout.size(); i++)
        {
            semantic_name = input_layout[i].SemanticName;
            const auto& range = semantic_siz_map.at(semantic_name);
            memcpy_s(reinterpret_cast<char*>(&vertex_dst) + input_layout[i].AlignedByteOffset,
                        range.second - range.first,
                        reinterpret_cast<const char*>(&vertex_src) + range.first,
                        range.second - range.first);
        }
    }
}

namespace toy::geometry
{
    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_sphere(float radius, uint32_t levels, uint32_t slices, const DirectX::XMFLOAT4 & color)
    {
        using namespace DirectX;

        MeshData<VertexType, IndexType> meshData;
        uint32_t vertexCount = 2 + (levels - 1) * (slices + 1);
        uint32_t indexCount = 6 * (levels - 1) * slices;
        meshData.vertices.resize(vertexCount);
        meshData.indices.resize(indexCount);

        internal::VertexData vertexData;
        IndexType vIndex = 0, iIndex = 0;

        float phi = 0.0f, theta = 0.0f;
        float per_phi = XM_PI / float(levels);
        float per_theta = XM_2PI / float(slices);
        float x, y, z;

        // Push into top point
        vertexData = { XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
        internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);

        for (uint32_t i = 1; i < levels; ++i)
        {
            phi = per_phi * float(i);
            // 需要slices + 1个顶点是因为 起点和终点需为同一点，但纹理坐标值不一致
            for (uint32_t j = 0; j <= slices; ++j)
            {
                theta = per_theta * float(j);
                x = radius * sinf(phi) * cosf(theta);
                y = radius * cosf(phi);
                z = radius * sinf(phi) * sinf(theta);
                // 计算出局部坐标、法向量、Tangent向量和纹理坐标
                XMFLOAT3 pos = XMFLOAT3(x, y, z);
                XMFLOAT3 normal{};
                XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));

                vertexData = { pos, normal, XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(theta / XM_2PI, phi / XM_PI) };
                internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);
            }
        }

        // 放入底端点
        vertexData = { XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),
                        XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 1.0f) };
        internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);


        // 放入索引
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                meshData.indices[iIndex++] = 0;
                meshData.indices[iIndex++] = j % (slices + 1) + 1;
                meshData.indices[iIndex++] = j;
            }
        }


        for (uint32_t i = 1; i < levels - 1; ++i)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                meshData.indices[iIndex++] = (i - 1) * (slices + 1) + j;
                meshData.indices[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
                meshData.indices[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

                meshData.indices[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
                meshData.indices[iIndex++] = i * (slices + 1) + j;
                meshData.indices[iIndex++] = (i - 1) * (slices + 1) + j;
            }
        }

        // 逐渐放入索引
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                meshData.indices[iIndex++] = (levels - 2) * (slices + 1) + j;
                meshData.indices[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
                meshData.indices[iIndex++] = (levels - 1) * (slices + 1) + 1;
            }
        }

        return meshData;
    }

    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_box(float width, float height, float depth, const DirectX::XMFLOAT4 & color)
    {
        using namespace DirectX;

        MeshData<VertexType, IndexType> meshData;
        meshData.vertices.resize(24);

        internal::VertexData vertexDataArr[24];
        float w2 = width / 2, h2 = height / 2, d2 = depth / 2;

        // 右面(+X面)
        vertexDataArr[0].pos = XMFLOAT3(w2, -h2, -d2);
        vertexDataArr[1].pos = XMFLOAT3(w2, h2, -d2);
        vertexDataArr[2].pos = XMFLOAT3(w2, h2, d2);
        vertexDataArr[3].pos = XMFLOAT3(w2, -h2, d2);
        // 左面(-X面)
        vertexDataArr[4].pos = XMFLOAT3(-w2, -h2, d2);
        vertexDataArr[5].pos = XMFLOAT3(-w2, h2, d2);
        vertexDataArr[6].pos = XMFLOAT3(-w2, h2, -d2);
        vertexDataArr[7].pos = XMFLOAT3(-w2, -h2, -d2);
        // 顶面(+Y面)
        vertexDataArr[8].pos = XMFLOAT3(-w2, h2, -d2);
        vertexDataArr[9].pos = XMFLOAT3(-w2, h2, d2);
        vertexDataArr[10].pos = XMFLOAT3(w2, h2, d2);
        vertexDataArr[11].pos = XMFLOAT3(w2, h2, -d2);
        // 底面(-Y面)
        vertexDataArr[12].pos = XMFLOAT3(w2, -h2, -d2);
        vertexDataArr[13].pos = XMFLOAT3(w2, -h2, d2);
        vertexDataArr[14].pos = XMFLOAT3(-w2, -h2, d2);
        vertexDataArr[15].pos = XMFLOAT3(-w2, -h2, -d2);
        // 背面(+Z面)
        vertexDataArr[16].pos = XMFLOAT3(w2, -h2, d2);
        vertexDataArr[17].pos = XMFLOAT3(w2, h2, d2);
        vertexDataArr[18].pos = XMFLOAT3(-w2, h2, d2);
        vertexDataArr[19].pos = XMFLOAT3(-w2, -h2, d2);
        // 正面(-Z面)
        vertexDataArr[20].pos = XMFLOAT3(-w2, -h2, -d2);
        vertexDataArr[21].pos = XMFLOAT3(-w2, h2, -d2);
        vertexDataArr[22].pos = XMFLOAT3(w2, h2, -d2);
        vertexDataArr[23].pos = XMFLOAT3(w2, -h2, -d2);

        for (UINT i = 0; i < 4; ++i)
        {
            // 右面(+X面)
            vertexDataArr[i].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
            vertexDataArr[i].tangent = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
            vertexDataArr[i].color = color;
            // 左面(-X面)
            vertexDataArr[i + 4].normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);
            vertexDataArr[i + 4].tangent = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
            vertexDataArr[i + 4].color = color;
            // 顶面(+Y面)
            vertexDataArr[i + 8].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
            vertexDataArr[i + 8].tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexDataArr[i + 8].color = color;
            // 底面(-Y面)
            vertexDataArr[i + 12].normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
            vertexDataArr[i + 12].tangent = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            vertexDataArr[i + 12].color = color;
            // 背面(+Z面)
            vertexDataArr[i + 16].normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
            vertexDataArr[i + 16].tangent = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            vertexDataArr[i + 16].color = color;
            // 正面(-Z面)
            vertexDataArr[i + 20].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
            vertexDataArr[i + 20].tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexDataArr[i + 20].color = color;
        }

        for (uint32_t i = 0; i < 6; ++i)
        {
            vertexDataArr[i * 4].tex = XMFLOAT2(0.0f, 1.0f);
            vertexDataArr[i * 4 + 1].tex = XMFLOAT2(0.0f, 0.0f);
            vertexDataArr[i * 4 + 2].tex = XMFLOAT2(1.0f, 0.0f);
            vertexDataArr[i * 4 + 3].tex = XMFLOAT2(1.0f, 1.0f);
        }

        for (uint32_t i = 0; i < 24; ++i)
        {
            internal::insert_vertex_element(meshData.vertices[i], vertexDataArr[i]);
        }

        meshData.indices = {
            0, 1, 2, 2, 3, 0,		// right face(+X face)
            4, 5, 6, 6, 7, 4,		// left face(-X face)
            8, 9, 10, 10, 11, 8,	// top face(+Y face)
            12, 13, 14, 14, 15, 12,	// bottom face(-Y face)
            16, 17, 18, 18, 19, 16, // back face(+Z face)
            20, 21, 22, 22, 23, 20	// front face(-Z face)
        };

        return meshData;
    }

    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_cone(float radius, float height, uint32_t slices, const DirectX::XMFLOAT4& color)
    {
        using namespace DirectX;
        auto meshData = create_cone_no_cap<VertexType, IndexType>(radius, height, slices, color);

        uint32_t vertexCount = 3 * slices + 1;
        uint32_t indexCount = 6 * slices;
        meshData.vertices.resize(vertexCount);
        meshData.indices.resize(indexCount);

        float h2 = height / 2.0f;
        float theta = 0.0f;
        float per_theta = XM_2PI / slices;
        uint32_t iIndex = 3 * slices;
        uint32_t vIndex = 2 * slices;
        internal::VertexData vertexData;

        // 放入圆锥底面顶点
        for (uint32_t i = 0; i < slices; ++i)
        {
            theta = i * per_theta;
            vertexData = { XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)), XMFLOAT3(0.0f, -1.0f, 0.0f),
                            XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f) };
            internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);
        }
        // 放入圆锥底面圆心
        vertexData = { XMFLOAT3(0.0f, -h2, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),
                        XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.5f, 0.5f) };
        internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);

        // 放入索引
        uint32_t offset = 2 * slices;
        for (uint32_t i = 0; i < slices; ++i)
        {
            meshData.indices[iIndex++] = offset + slices;
            meshData.indices[iIndex++] = offset + i % slices;
            meshData.indices[iIndex++] = offset + (i + 1) % slices;
        }

        return meshData;
    }

    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_cone_no_cap(float radius, float height, uint32_t slices, const DirectX::XMFLOAT4& color)
    {
        using namespace DirectX;

        MeshData<VertexType, IndexType> meshData;
        uint32_t vertexCount = 2 * slices;
        uint32_t indexCount = 3 * slices;
        meshData.vertices.resize(vertexCount);
        meshData.indices.resize(indexCount);

        float h2 = height / 2;
        float theta = 0.0f;
        float per_theta = XM_2PI / slices;
        float len = std::sqrt(height * height + radius * radius);
        uint32_t iIndex = 0;
        uint32_t vIndex = 0;
        internal::VertexData vertexData;

        // 放入圆锥尖端顶点(每个顶点位置相同，但包含不同的法向量和切线向量)
        for (uint32_t i = 0; i < slices; ++i)
        {
            theta = i * per_theta + per_theta / 2;
            vertexData = { XMFLOAT3(0.0f, h2, 0.0f), XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len),
                            XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(0.5f, 0.5f) };
            internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);
        }

        // 放入圆锥侧面底部顶点
        for (uint32_t i = 0; i < slices; ++i)
        {
            theta = i * per_theta;
            vertexData = { XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)), XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len),
                            XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f), color, XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f) };
            internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);
        }

        // 放入索引
        for (uint32_t i = 0; i < slices; ++i)
        {
            meshData.indices[iIndex++] = i;
            meshData.indices[iIndex++] = slices + (i + 1) % slices;
            meshData.indices[iIndex++] = slices + i % slices;
        }

        return meshData;
    }

    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_2d_show(const DirectX::XMFLOAT2& center, const DirectX::XMFLOAT2 & scale, const DirectX::XMFLOAT4 & color)
    {
        return create_2d_show<VertexType, IndexType>(center.x, center.y, scale.x, scale.y, color);
    }

    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_2d_show(float centerX, float centerY, float scaleX, float scaleY, const DirectX::XMFLOAT4 & color)
    {
        using namespace DirectX;

        MeshData<VertexType, IndexType> meshData;
        meshData.vertexVec.resize(4);

        internal::VertexData vertexData;
        uint32_t vIndex = 0;

        vertexData = { XMFLOAT3(centerX - scaleX, centerY - scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 1.0f) };
        internal::insert_vertex_element(meshData.vertexVec[vIndex++], vertexData);

        vertexData = { XMFLOAT3(centerX - scaleX, centerY + scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
        internal::insert_vertex_element(meshData.vertexVec[vIndex++], vertexData);

        vertexData = { XMFLOAT3(centerX + scaleX, centerY + scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 0.0f) };
        internal::insert_vertex_element(meshData.vertexVec[vIndex++], vertexData);

        vertexData = { XMFLOAT3(centerX + scaleX, centerY - scaleY, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 1.0f) };
        internal::insert_vertex_element(meshData.vertexVec[vIndex++], vertexData);

        meshData.indexVec = { 0, 1, 2, 2, 3, 0 };
        return meshData;
    }

    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_plane(const DirectX::XMFLOAT2 & planeSize,
                                                        const DirectX::XMFLOAT2 & maxTexCoord, const DirectX::XMFLOAT4 & color)
    {
        return create_plane<VertexType, IndexType>(planeSize.x, planeSize.y, maxTexCoord.x, maxTexCoord.y, color);
    }

    template<class VertexType, class IndexType>
    MeshData<VertexType, IndexType> create_plane(float width, float depth, float texU, float texV, const DirectX::XMFLOAT4 & color)
    {
        using namespace DirectX;

        MeshData<VertexType, IndexType> meshData;
        meshData.vertices.resize(4);

        internal::VertexData vertexData;
        UINT vIndex = 0;

        vertexData = { XMFLOAT3(-width / 2, 0.0f, -depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, texV) };
        internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);

        vertexData = { XMFLOAT3(-width / 2, 0.0f, depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
        internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);

        vertexData = { XMFLOAT3(width / 2, 0.0f, depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(texU, 0.0f) };
        internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);

        vertexData = { XMFLOAT3(width / 2, 0.0f, -depth / 2), XMFLOAT3(0.0f, 1.0f, 0.0f),
                        XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), color, XMFLOAT2(texU, texV) };
        internal::insert_vertex_element(meshData.vertices[vIndex++], vertexData);

        meshData.indices = { 0, 1, 2, 2, 3, 0 };
        return meshData;
    }
}
















