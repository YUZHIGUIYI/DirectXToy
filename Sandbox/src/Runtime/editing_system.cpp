//
// Created by ZZK on 2024/3/22.
//

#include <Sandbox/Runtime/editing_system.h>
#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/scene_graph.h>
#include <Toy/Runtime/renderer.h>

namespace toy::editor
{
    EditingSystem::EditingSystem()
    {
        // Initialize editor camera
        using namespace DirectX;
        auto&& renderer = core::get_subsystem<runtime::Renderer>();
        auto&& scene_graph = core::get_subsystem<runtime::SceneGraph>();
        auto editor_camera = std::make_shared<FirstPersonCamera>();
        float width = 1600.0f;
        float height = 900.0f;
        editor_camera->set_viewport(0.0f, 0.0f, width, height);
        editor_camera->set_frustum(XM_PI / 3.0f, width / height, 0.5f, 360.0f);
        editor_camera->look_at(XMFLOAT3{-60.0f, 10.0f, 2.5f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 1.0f, 0.0f });

        m_editor_camera_entity = scene_graph.create_entity("EditorCamera");
        auto&& camera_component = m_editor_camera_entity.add_component<CameraComponent>();
        camera_component.camera = std::move(editor_camera);
        camera_component.camera_type = CameraType::FirstPersonCamera;
        DX_INFO("Editor camera entity id: {}", static_cast<uint32_t>(m_editor_camera_entity.entity_inst));

        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_diffuse.jpg");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_normal.jpg");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_metallic.jpg");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_roughness.jpg");

        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_A.tga", true, 1);
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_N.tga");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_M.tga");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_R.tga");

        // Initialize skybox
        auto skybox_entity = scene_graph.create_entity("Skybox");
        model::ModelManager::get().create_from_geometry("SkyboxCube", geometry::create_box(static_cast<uint32_t>(skybox_entity.entity_inst)));
        skybox_entity.add_component<TransformComponent>();
        auto& skybox_mesh = skybox_entity.add_component<StaticMeshComponent>();
        skybox_mesh.model_asset = model::ModelManager::get().get_model("SkyboxCube");
        skybox_mesh.is_skybox = true;
        DX_INFO("Skybox entity id: {}", static_cast<uint32_t>(skybox_entity.entity_inst));

        // Initialize cerberus
        auto cerberus_entity = scene_graph.create_entity("Cerberus");
        model::ModelManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Cerberus_LP.fbx", static_cast<uint32_t>(cerberus_entity.entity_inst));
        auto& cerberus_transform = cerberus_entity.add_component<TransformComponent>();
        cerberus_transform.transform.set_scale(0.3f, 0.3f, 0.3f);
        cerberus_transform.transform.set_rotation(XM_PI / 2.0f, XM_PI, XM_PI / 2.0f);
        auto& cerberus_mesh = cerberus_entity.add_component<StaticMeshComponent>();
        cerberus_mesh.model_asset = model::ModelManager::get().get_model(DXTOY_HOME "data/models/Cerberus/Cerberus_LP.fbx");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(model::MaterialSemantics::DiffuseMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_A.tga");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(model::MaterialSemantics::NormalMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_N.tga");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(model::MaterialSemantics::MetalnessMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_M.tga");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(model::MaterialSemantics::RoughnessMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_R.tga");
        DX_INFO("Cerberus entity id: {}", static_cast<uint32_t>(cerberus_entity.entity_inst));

        // Initialize directional light
        auto directional_light_entity = scene_graph.create_entity("GlobalDirectionalLight");
        auto&& directional_light_component = directional_light_entity.add_component<DirectionalLightComponent>();
        directional_light_component.position = DirectX::XMFLOAT3{ -15.0f, 55.0f, -10.0f };
        directional_light_component.target = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
        directional_light_component.color = DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f };
        directional_light_component.intensity = 1.0f;
        DX_INFO("Global directional light entity id: {}", static_cast<uint32_t>(directional_light_entity.entity_inst));
    }

    void EditingSystem::select(uint32_t entity_id)
    {
        auto&& scene_graph = core::get_subsystem<runtime::SceneGraph>();
        auto&& renderer = core::get_subsystem<runtime::Renderer>();
        m_selected_entity = scene_graph.get_entity(entity_id);
        renderer.reset_selected_entity(m_selected_entity);
    }

    void EditingSystem::select(const EntityWrapper &entity_wrapper)
    {
        auto&& renderer = core::get_subsystem<runtime::Renderer>();
        m_selected_entity = entity_wrapper;
        renderer.reset_selected_entity(m_selected_entity);
    }

    void EditingSystem::unselect()
    {
        auto&& renderer = core::get_subsystem<runtime::Renderer>();
        m_selected_entity = {};
        renderer.reset_selected_entity(m_selected_entity);
    }

    EntityWrapper &EditingSystem::get_editor_camera_entity()
    {
        return m_editor_camera_entity;
    }

    EntityWrapper &EditingSystem::get_selected_entity()
    {
        return m_selected_entity;
    }

    GizmoSnap &EditingSystem::get_gizmo_snap()
    {
        return m_gizmo_snap;
    }

    ImGuizmo::OPERATION &EditingSystem::get_gizmo_operation()
    {
        return m_gizmo_operation;
    }

    float &EditingSystem::get_mouse_speed()
    {
        return m_mouse_speed;
    }

    float &EditingSystem::get_mouse_sensitivity_x()
    {
        return m_mouse_sensitivity_x;
    }

    float &EditingSystem::get_mouse_sensitivity_y()
    {
        return m_mouse_sensitivity_y;
    }
}