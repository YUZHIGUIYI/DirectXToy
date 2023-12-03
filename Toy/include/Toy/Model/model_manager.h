//
// Created by ZZK on 2023/5/31.
//

#pragma once

#include <Toy/Model/mesh_data.h>
#include <Toy/Model/material.h>
#include <Toy/Geometry/geometry.h>

namespace toy::model
{
    struct Model
    {
        Model() = default;
        ~Model() = default;

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        Model(Model&&) noexcept = default;
        Model& operator=(Model&&) = default;

        std::vector<Material> materials;
        std::vector<MeshData> meshes;
        DirectX::BoundingBox bounding_box;

        static void create_from_file(Model& model, ID3D11Device* device, std::string_view file_name, uint32_t entity_id = 1);
        static void create_from_geometry(Model& model, ID3D11Device* device, const geometry::GeometryData& data, bool is_dynamic = false);

        void set_debug_object_name(std::string_view name);
    };

    class ModelManager
    {
    public:
        ModelManager() = default;
        ~ModelManager() = default;

        ModelManager(ModelManager&) = delete;
        ModelManager& operator=(const ModelManager&) = delete;
        ModelManager(ModelManager&&) noexcept = default;
        ModelManager& operator=(ModelManager&&) = default;

        void init(ID3D11Device* device);

        Model* create_from_file(std::string_view file_name, uint32_t entity_id = 1);
        Model* create_from_file(std::string_view name, std::string_view file_name, uint32_t entity_id = 1);
        Model* create_from_geometry(std::string_view name, const geometry::GeometryData& data, bool is_dynamic = false);

        [[nodiscard]] const Model* get_model(std::string_view name) const;
        Model* get_model(std::string_view name);

        // Singleton
        static ModelManager &get();

    private:
        com_ptr<ID3D11Device> m_device_;
        com_ptr<ID3D11DeviceContext> m_device_context_;
        std::unordered_map<size_t, Model> m_models;
    };
}
































