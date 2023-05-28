//
// Created by ZZK on 2023/5/24.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    class render_object_c
    {
    public:
        render_object_c();

        // Object transform
        transform_c& get_transform();
        const transform_c& get_transform() const;

        // Set buffer
        template<typename VertexType, class IndexType>
        void set_buffer(ID3D11Device *device, const geometry::MeshData<VertexType, IndexType> &mesh_data);
        // Set texture
        void set_texture(ID3D11ShaderResourceView *texture);
        // TODO: set material
        void set_material(const Material& material);

        // Draw
        void draw(ID3D11DeviceContext *device_context, basic_effect_c& effect);

        // Set debug object name
        void set_debug_object_name(const std::string& name);

    public:
        transform_c class_transform;
        Material class_material;
        com_ptr<ID3D11ShaderResourceView> class_texture;
        com_ptr<ID3D11Buffer> class_vertex_buffer;
        com_ptr<ID3D11Buffer> class_index_buffer;
        uint32_t class_vertex_stride;
        uint32_t class_index_count;
    };

    template<typename VertexType, typename IndexType>
    void render_object_c::set_buffer(ID3D11Device *device, const geometry::MeshData<VertexType, IndexType> &mesh_data)
    {
        class_vertex_buffer.Reset();
        class_index_buffer.Reset();

        // Set vertex buffer description
        class_vertex_stride = sizeof(VertexType);
        D3D11_BUFFER_DESC vertex_desc{};
        vertex_desc.Usage = D3D11_USAGE_IMMUTABLE;
        vertex_desc.ByteWidth = static_cast<uint32_t>(mesh_data.vertices.size() * class_vertex_stride);
        vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_desc.CPUAccessFlags = 0;
        // Create vertex buffer
        D3D11_SUBRESOURCE_DATA init_data{};
        init_data.pSysMem = mesh_data.vertices.data();
        device->CreateBuffer(&vertex_desc, &init_data, class_vertex_buffer.GetAddressOf());

        // Set index buffer description
        class_index_count = mesh_data.indices.size();
        D3D11_BUFFER_DESC index_desc{};
        index_desc.Usage = D3D11_USAGE_IMMUTABLE;
        index_desc.ByteWidth = static_cast<uint32_t>(class_index_count * sizeof(IndexType));
        index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        index_desc.CPUAccessFlags = 0;
        // Create index buffer
        init_data.pSysMem = mesh_data.indices.data();
        device->CreateBuffer(&index_desc, &init_data, class_index_buffer.GetAddressOf());
    }
}
