//
// Created by ZZK on 2023/10/1.
//

#include <Sandbox/pbr_viewer.h>
#include <Sandbox/file_dialog.h>
#include <Toy/Renderer/taa_settings.h>
#include <Toy/Renderer/cascaded_shadow_manager.h>

#include <IconsFontAwesome6.h>

namespace toy::viewer
{
    // Alias
    using Vec3f = std::array<float, 3>;

    // Color to clear rtv
    const float s_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Update transform
    std::variant<std::true_type, std::false_type> bool_variant(bool cond)
    {
        if (cond) return std::true_type{};
        else return std::false_type{};
    }

    void update_transform(TransformComponent& transform_comp, Vec3f& translation, Vec3f& rotation, Vec3f& scale, ImGuizmo::OPERATION operation_type)
    {
        auto change_translation = bool_variant(operation_type == ImGuizmo::OPERATION::TRANSLATE);
        auto change_rotation = bool_variant(operation_type == ImGuizmo::OPERATION::ROTATE);
        auto change_scale = bool_variant(operation_type == ImGuizmo::OPERATION::SCALE);
        std::visit([&transform_comp, &translation, &rotation, &scale] (auto translation_changed, auto rotation_changed, auto scale_changed) {
            if constexpr (translation_changed)
            {
                transform_comp.transform.set_position(translation[0], translation[1], translation[2]);
            } else if constexpr (rotation_changed)
            {
                for (auto& each_rotation : rotation)
                {
                    each_rotation = DirectX::XMConvertToRadians(each_rotation);
                }
                transform_comp.transform.set_rotation(rotation[0], rotation[1], rotation[2]);
            } else if constexpr (scale_changed)
            {
                for (auto& each_scale : scale)
                {
                    each_scale = std::clamp(each_scale, 0.1f, 10.0f);
                }
                transform_comp.transform.set_scale(scale[0], scale[1], scale[2]);
            }
        }, change_translation, change_rotation, change_scale);
    }

    PBRViewer::PBRViewer(std::string_view viewer_name)
            : m_viewer_name(viewer_name)
    {
    }

    PBRViewer::~PBRViewer() = default;

    bool PBRViewer::is_viewer_size_changed()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(m_viewer_name.data());

        auto viewport_size     = ImGui::GetContentRegionAvail();
        auto viewer_min_region = ImGui::GetWindowContentRegionMin();
        auto viewer_max_region = ImGui::GetWindowContentRegionMax();
        auto viewer_offset     = ImGui::GetWindowPos();

        bool viewport_size_changed = (static_cast<int32_t>(viewport_size.x) != m_viewer_spec.width
                                        || static_cast<int32_t>(viewport_size.y) != m_viewer_spec.height);
        ImGui::End();
        ImGui::PopStyleVar();

        if (viewport_size_changed)
        {
            m_viewer_spec.width = static_cast<int32_t>(viewport_size.x);
            m_viewer_spec.height = static_cast<int32_t>(viewport_size.y);

            m_viewer_spec.lower_bound = DirectX::XMFLOAT2{ viewer_offset.x + viewer_min_region.x, viewer_offset.y + viewer_min_region.y };
            m_viewer_spec.upper_bound = DirectX::XMFLOAT2{ viewer_offset.x + viewer_max_region.x, viewer_offset.y + viewer_max_region.y };
        }

        return viewport_size_changed;
    }

    void PBRViewer::on_attach(toy::d3d_application_c *app)
    {
        m_d3d_app = app;

        // Initialize texture manager and model manager
        auto d3d_device = m_d3d_app->get_device();
        model::TextureManager::get().init(d3d_device);
        model::ModelManager::get().init(d3d_device);

        // Initialize render states
        RenderStates::init(d3d_device);

        // Initialize effects
        ShadowEffect::get().init(d3d_device);
        DeferredPBREffect::get().init(d3d_device);
        SimpleSkyboxEffect::get().init(d3d_device);
        PreProcessEffect::get().init(d3d_device);
        TAAEffect::get().init(d3d_device);

        // Initialize shadow
        CascadedShadowManager::get().init(d3d_device);

        // Initialize resource
        init_resource();

        // Initialize buffers
        resize_buffers(m_viewer_spec.width, m_viewer_spec.height);
    }

    void PBRViewer::on_detach()
    {
    }

    void PBRViewer::on_resize()
    {
        using namespace DirectX;

        if (!is_viewer_size_changed())
        {
            return;
        }

        // Recreate resources
        auto width = m_viewer_spec.width;
        auto height = m_viewer_spec.height;
        auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->set_frustum(XM_PI / 3.0f, aspect_ratio, 0.5f, 300.0f);
        m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
        DeferredPBREffect::get().set_proj_matrix(m_camera->get_proj_xm(false));
        DeferredPBREffect::get().set_camera_near_far(m_camera->get_near_z(), m_camera->get_far_z());
        TAAEffect::get().set_viewer_size(width, height);
        TAAEffect::get().set_camera_near_far(m_camera->get_near_z(), m_camera->get_far_z());

        resize_buffers(width, height);
    }

    void PBRViewer::on_ui_menu()
    {
        bool load_file = false;

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load", "Ctrl+O"))
            {
                load_file = true;
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }

        if (m_busy) return;
        auto glfw_window = m_d3d_app->get_glfw_window();
        if (DX_INPUT::is_key_pressed(glfw_window, key::O) && DX_INPUT::is_key_pressed(glfw_window, key::LeftControl))
        {
            load_file = true;
        }

        if (load_file)
        {
            auto filename = FileDialog::window_open_file_dialog(glfw_window, "Load glTF | HDR | FBX",
                                                                "glTF(.gltf, .glb), HDR(.hdr), FBX(.fbx)|*.gltf;*.hdr;*.fbx");
            on_file_drop(filename);
        }
    }

    void PBRViewer::on_file_drop(std::string_view filename)
    {
        if (m_busy) return;

        m_busy.store(true);
        const std::string extension = std::filesystem::path(filename).extension().string();
        auto name = std::filesystem::path(filename).filename();
        if (extension == ".gltf" || extension == ".glb" || extension == ".fbx")
        {
            // TODO: create scene
            model::ModelManager::get().create_from_file(filename);

            auto new_entity = m_editor_scene->create_entity(name.string());
            auto& new_transform = new_entity.add_component<TransformComponent>();
            new_transform.transform.set_scale(0.1f, 0.1f, 0.1f);
            auto& new_mesh = new_entity.add_component<StaticMeshComponent>();
            new_mesh.model_asset = model::ModelManager::get().get_model(filename);

        } else if (extension == ".hdr")
        {
            auto d3d_device = m_d3d_app->get_device();
            auto d3d_device_context = m_d3d_app->get_device_context();
            PreProcessEffect::get().compute_cubemap(d3d_device, d3d_device_context, filename);
            PreProcessEffect::get().compute_sp_env_map(d3d_device, d3d_device_context);
            PreProcessEffect::get().compute_irradiance_map(d3d_device, d3d_device_context);
            PreProcessEffect::get().compute_brdf_lut(d3d_device, d3d_device_context);
        }
        if (!name.empty())
        {
            DX_INFO("Load asset file {} successfully", name);
        }
        m_busy.store(false);
    }

    void PBRViewer::on_gizmo_render(toy::Entity &selected_entity)
    {
        // TODO: Begin ImGui Gizmos
        static ImGuizmo::OPERATION gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
        bool shift_pressed = DX_INPUT::is_key_pressed(m_d3d_app->get_glfw_window(), key::LeftShift);
        if (shift_pressed && !ImGuizmo::IsUsing())
        {
            if (DX_INPUT::is_key_pressed(m_d3d_app->get_glfw_window(), key::F1))
            {
                gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
            } else if (DX_INPUT::is_key_pressed(m_d3d_app->get_glfw_window(), key::F2))
            {
                gizmo_type = ImGuizmo::OPERATION::SCALE;
            } else if (DX_INPUT::is_key_pressed(m_d3d_app->get_glfw_window(), key::F3))
            {
                gizmo_type = ImGuizmo::OPERATION::ROTATE;
            }
        }
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(m_viewer_spec.lower_bound.x, m_viewer_spec.lower_bound.y,
                            m_viewer_spec.upper_bound.x - m_viewer_spec.lower_bound.x, m_viewer_spec.upper_bound.y - m_viewer_spec.lower_bound.y);

        auto camera_proj = m_camera->get_proj_xm();
        auto camera_view = m_camera->get_view_xm();

        auto& transform_component = selected_entity.get_component<TransformComponent>();
        auto transform_matrix = transform_component.transform.get_local_to_world_matrix_xm();

        bool keep_snap = DX_INPUT::is_key_pressed(m_d3d_app->get_glfw_window(), key::LeftControl);
        float snap_value = 5.0f;
        float snap_values[3] = { snap_value, snap_value, snap_value };
        ImGuizmo::Manipulate((float *)&camera_view, (float *)&camera_proj, (ImGuizmo::OPERATION)gizmo_type, ImGuizmo::LOCAL, (float *)&transform_matrix,
                                nullptr, keep_snap ? snap_values : nullptr);

        if (ImGuizmo::IsUsing())
        {
            std::array<float, 3> translation_vector{};
            std::array<float, 3> rotation_vector{};
            std::array<float, 3> scale_vector{};
            ImGuizmo::DecomposeMatrixToComponents((float *)&transform_matrix, translation_vector.data(), rotation_vector.data(), scale_vector.data());
            update_transform(transform_component, translation_vector, rotation_vector, scale_vector, gizmo_type);
        }
    }

    void PBRViewer::on_ui_render()
    {
        // Rendering viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
        ImGui::Begin(m_viewer_name.data());

        m_viewer_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
        m_viewer_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow);

        ImGui::Image(m_viewer_buffer->get_shader_resource(), ImGui::GetContentRegionAvail());

        // Rendering Gizmo
        if (m_viewer_focused && m_selected_entity.is_valid())
        {
            on_gizmo_render(m_selected_entity);
        }

        ImGui::End();
        ImGui::PopStyleVar();

        // Rendering scene hierarchy panel
        m_scene_hierarchy_panel.on_ui_render();
    }

    void PBRViewer::init_resource()
    {
        // Initialize camera and controller
        using namespace DirectX;
        auto camera = std::make_shared<first_person_camera_c>();
        m_camera = camera;

        m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(m_viewer_spec.width), static_cast<float>(m_viewer_spec.height));
        m_camera->set_frustum(XM_PI / 3.0f, m_viewer_spec.get_aspect_ratio(), 0.5f, 300.0f);
        camera->look_at(XMFLOAT3(-60.0f, 10.0f, 2.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

        auto light_camera = std::make_shared<first_person_camera_c>();
        m_light_camera = light_camera;

        m_light_camera->set_viewport(0.0f, 0.0f, static_cast<float>(m_viewer_spec.width), static_cast<float>(m_viewer_spec.height));
        m_light_camera->set_frustum(XM_PI / 3.0f, 1.0f, 0.1f, 1200.0f);
        light_camera->look_at(XMFLOAT3{ -15.0f, 55.0f, -10.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 1.0f, 0.0f });

        m_camera_controller.init(camera.get());
        m_camera_controller.set_move_speed(25.0f);

        // Initialize effects
        auto&& cascade_shadow_manager = CascadedShadowManager::get();
        auto&& deferred_pbr_effect = DeferredPBREffect::get();
        deferred_pbr_effect.set_view_matrix(camera->get_view_xm());
        //// reverse z - currently prohibited
        deferred_pbr_effect.set_proj_matrix(camera->get_proj_xm(false));
        deferred_pbr_effect.set_camera_near_far(m_camera->get_near_z(), m_camera->get_far_z());

        deferred_pbr_effect.set_pcf_kernel_size(cascade_shadow_manager.pcf_kernel_size);
        deferred_pbr_effect.set_pcf_depth_offset(cascade_shadow_manager.pcf_depth_offset);
        deferred_pbr_effect.set_shadow_size(cascade_shadow_manager.shadow_size);
        deferred_pbr_effect.set_cascade_blend_enabled(cascade_shadow_manager.blend_between_cascades);
        deferred_pbr_effect.set_cascade_blend_area(cascade_shadow_manager.blend_between_cascades_range);
        deferred_pbr_effect.set_cascade_levels(cascade_shadow_manager.cascade_levels);
        deferred_pbr_effect.set_cascade_interval_selection_enabled(static_cast<bool>(cascade_shadow_manager.selected_cascade_selection));
        deferred_pbr_effect.set_pcf_derivatives_offset_enabled(cascade_shadow_manager.derivative_based_offset);

        TAAEffect::get().set_viewer_size(m_viewer_spec.width, m_viewer_spec.height);
        TAAEffect::get().set_camera_near_far(m_camera->get_near_z(), m_camera->get_far_z());

        ShadowEffect::get().set_view_matrix(m_light_camera->get_view_xm());

        // Skybox texture
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/Clouds.dds");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/bricks.dds");

        // Initialize models
        model::ModelManager::get().create_from_geometry("skyboxCube", geometry::create_box());
        // TODO: Debug - start
        model::ModelManager::get().create_from_geometry("simpleBall", geometry::create_sphere());
        model::ModelManager::get().create_from_geometry("simpleCylinder", geometry::create_cylinder(1.0f, 2.0f, 40, 20));
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_diffuse.jpg");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_normal.jpg");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_metallic.jpg");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_roughness.jpg");

        auto d3d_device = m_d3d_app->get_device();
        auto d3d_device_context = m_d3d_app->get_device_context();
        PreProcessEffect::get().compute_cubemap(d3d_device, d3d_device_context, DXTOY_HOME "data/textures/environment.hdr");
        PreProcessEffect::get().compute_sp_env_map(d3d_device, d3d_device_context);
        PreProcessEffect::get().compute_irradiance_map(d3d_device, d3d_device_context);
        PreProcessEffect::get().compute_brdf_lut(d3d_device, d3d_device_context);

        model::ModelManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Cerberus_LP.fbx");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_A.tga", true, 1);
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_N.tga");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_M.tga");
        model::TextureManager::get().create_from_file(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_R.tga");
        // TODO: Debug - end

        // ECS
        m_editor_scene = std::make_shared<Scene>();

        auto skybox_entity = m_editor_scene->create_entity("Skybox");
        skybox_entity.add_component<TransformComponent>();
        auto& skybox_mesh = skybox_entity.add_component<StaticMeshComponent>();
        skybox_mesh.model_asset = model::ModelManager::get().get_model("skyboxCube");
        skybox_mesh.is_skybox = true;
        skybox_mesh.model_asset->materials[0].set<std::string>("$Skybox", DXTOY_HOME "data/textures/Clouds.dds");

        //// TODO: Debug - Test entity one
        using namespace toy::model;
        auto test_entity = m_editor_scene->create_entity("SphereWithTexture");
        auto& test_transform = test_entity.add_component<TransformComponent>();
        test_transform.transform.set_position(-15.0f, 15.0f, 10.0f);
        test_transform.transform.set_scale(10.0f, 10.0f, 10.0f);
        auto& test_mesh = test_entity.add_component<StaticMeshComponent>();
        test_mesh.model_asset = model::ModelManager::get().get_model("simpleBall");
        test_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::DiffuseMap),
                                                                DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_diffuse.jpg");
        test_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::NormalMap),
                                                                DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_normal.jpg");
        test_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::MetalnessMap),
                                                                DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_metallic.jpg");
        test_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::RoughnessMap),
                                                                DXTOY_HOME "data/textures/cgaxis_brown_clay_tiles_4K/brown_clay_tiles_44_49_roughness.jpg");

        //// TODO: Debug: Test entity two
        auto cerberus_entity = m_editor_scene->create_entity("Cerberus");
        auto& cerberus_transform = cerberus_entity.add_component<TransformComponent>();
        cerberus_transform.transform.set_scale(0.3f, 0.3f, 0.3f);
        cerberus_transform.transform.set_rotation(XM_PI / 2.0f, XM_PI, XM_PI / 2.0f);

        auto& cerberus_mesh = cerberus_entity.add_component<StaticMeshComponent>();
        cerberus_mesh.model_asset = model::ModelManager::get().get_model(DXTOY_HOME "data/models/Cerberus/Cerberus_LP.fbx");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::DiffuseMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_A.tga");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::NormalMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_N.tga");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::MetalnessMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_M.tga");
        cerberus_mesh.model_asset->materials[0].set<std::string>(material_semantics_name(MaterialSemantics::RoughnessMap),
                                                                    DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_R.tga");

        //// TODO: Debug: Test entity three
        auto test_entity_two = m_editor_scene->create_entity("CylinderWithoutTexture");
        auto& test_transform_two = test_entity_two.add_component<TransformComponent>();
        test_transform_two.transform.set_scale(10.0f, 10.0f, 10.0f);
        auto& test_mesh_two = test_entity_two.add_component<StaticMeshComponent>();
        test_mesh_two.model_asset = model::ModelManager::get().get_model("simpleCylinder");

        //// TODO: Debug: Test entity four
        auto test_entity_floor = m_editor_scene->create_entity("FloorWithoutTexture");
        auto& test_floor_transform = test_entity_floor.add_component<TransformComponent>();
        test_floor_transform.transform.set_scale(80.0f, 1.0f, 80.0f);
        test_floor_transform.transform.set_position(0.0f, -25.0f, 0.0f);
        auto& test_floor_mesh = test_entity_floor.add_component<StaticMeshComponent>();
        test_floor_mesh.model_asset = model::ModelManager::get().get_model("skyboxCube");

        // TODO: Debug - end

        // TODO: camera component
        auto test_entity_camera = m_editor_scene->create_entity("FirstIlluminant");
        auto& test_camera_component = test_entity_camera.add_component<CameraComponent>();
        test_camera_component.camera = m_light_camera;
        auto& test_camera_transform = test_entity_camera.add_component<TransformComponent>();
        test_camera_transform.transform.set_scale(3.0f, 3.0f, 3.0f);
        test_camera_transform.transform.set_position(-15.0f, 55.0f, -10.0f);
        auto& test_camera_mesh = test_entity_camera.add_component<StaticMeshComponent>();
        test_camera_mesh.model_asset = model::ModelManager::get().get_model("skyboxCube");
        test_camera_mesh.is_camera = true;
        // TODO: end camera component

        m_scene_hierarchy_panel.set_context(m_editor_scene);
    }

    void PBRViewer::resize_buffers(uint32_t width, uint32_t height)
    {
        ID3D11Device* d3d_device = m_d3d_app->get_device();

        // Initialize resources for deferred rendering
        m_depth_buffer = std::make_unique<Depth2D>(d3d_device, width, height, DepthStencilBitsFlag::Depth_32Bits);

        // G-Buffer
        m_gbuffers.clear();
        // Albedo and metalness
        m_gbuffers.push_back(std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                            1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
        // Normal and roughness
        m_gbuffers.push_back(std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                                            1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
        // World position
        m_gbuffers.push_back(std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                                            1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
        // Motion vector
        m_gbuffers.push_back(std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16_UNORM,
                                                            1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));

        // Set G-Buffer resource list
        m_gbuffer_rtvs.resize(m_gbuffers.size(), 0);
        m_gbuffer_srvs.resize(4, 0);
        for (size_t i = 0; i < m_gbuffers.size(); ++i)
        {
            m_gbuffer_rtvs[i] = m_gbuffers[i]->get_render_target();
            m_gbuffer_srvs[i] = m_gbuffers[i]->get_shader_resource();
        }
        // Depth buffer srv for reading
        m_gbuffer_srvs.back() = m_depth_buffer->get_shader_resource();

        // Viewer buffer
        m_viewer_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);

        // TAA effect input
        m_history_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
        m_cur_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);

        // Shadow effect debug
        m_shadow_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
    }

    void PBRViewer::on_render(float dt)
    {
        // Cascaded shadow pass
        render_shadow();

        // Update
        on_update(dt);

        ID3D11Device* d3d_device = m_d3d_app->get_device();
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();

        // Geometry pass
        render_gbuffer();

        // Deferred PBR and TAA pass
        taa_pass();

        // Skybox pass
        render_skybox();

        // Deferred rendering debug
        float aspect_ratio = m_viewer_spec.get_aspect_ratio();

        if (ImGui::Begin("Albedo"))
        {
            static ID3D11ShaderResourceView* albedo_srv = model::TextureManager::get().get_texture(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_A.tga");
            ImVec2 winSize = ImGui::GetWindowSize();
            float smaller = (std::min)((winSize.x - 20) / aspect_ratio, winSize.y - 20);
            ImGui::Image(albedo_srv, ImVec2(smaller * aspect_ratio, smaller));
        }
        ImGui::End();

        if (ImGui::Begin("Normal"))
        {
            static ID3D11ShaderResourceView* normal_srv = model::TextureManager::get().get_texture(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_N.tga");
            ImVec2 winSize = ImGui::GetWindowSize();
            float smaller = (std::min)((winSize.x - 20.0f) / aspect_ratio, winSize.y - 20.0f);
            ImGui::Image(normal_srv, ImVec2(smaller * aspect_ratio, smaller));
        }
        ImGui::End();

        if (ImGui::Begin("Metalness"))
        {
            static ID3D11ShaderResourceView* metalness_srv = model::TextureManager::get().get_texture(DXTOY_HOME "data/models/Cerberus/Textures/Cerberus_M.tga");
            ImVec2 winSize = ImGui::GetWindowSize();
            float smaller = (std::min)((winSize.x - 20.0f) / aspect_ratio, winSize.y - 20.0f);
            ImGui::Image(metalness_srv, ImVec2(smaller * aspect_ratio, smaller));
        }
        ImGui::End();

        if (ImGui::Begin("Roughness"))
        {
            auto&& cascade_shadow_manager = CascadedShadowManager::get();
            static const std::array<const char *, 7> cascade_level_selections{
                "Level 1", "Level 2", "Level 3", "Level 4", "Level 5", "Level 6", "Level 7"
            };
            static int32_t cur_level_index = 0;
            ImGui::Combo("##1", &cur_level_index, cascade_level_selections.data(), cascade_shadow_manager.cascade_levels);
            if (cur_level_index >= cascade_shadow_manager.cascade_levels)
            {
                cur_level_index = cascade_shadow_manager.cascade_levels - 1;
            }

            ShadowEffect::get().render_depth_to_texture(d3d_device_context, cascade_shadow_manager.get_cascade_output(cur_level_index),
                                                        m_shadow_buffer->get_render_target(), cascade_shadow_manager.shadow_viewport);

            ImVec2 winSize = ImGui::GetWindowSize();
            float smaller = (std::min)((winSize.x - 20.0f) / aspect_ratio, winSize.y - 20.0f);
            ImGui::Image(m_shadow_buffer->get_shader_resource(), ImVec2(smaller * aspect_ratio, smaller));
        }
        ImGui::End();
    }

    void PBRViewer::on_update(float dt)
    {
        using namespace DirectX;

        GLFWwindow* glfw_window = m_d3d_app->get_glfw_window();
        // Update camera if viewer is focused and hovered
        if (m_viewer_focused && m_viewer_hovered)
        {
            m_camera_controller.update(dt, glfw_window);
        }

        DeferredPBREffect::get().set_view_matrix(m_camera->get_view_xm());
        DeferredPBREffect::get().set_camera_position(m_camera->get_position());
        DeferredPBREffect::get().set_viewer_size(m_viewer_spec.width, m_viewer_spec.height);

        // Update collision
        BoundingFrustum frustum{};
        BoundingFrustum::CreateFromMatrix(frustum, m_camera->get_proj_xm());
        frustum.Transform(frustum, m_camera->get_local_to_world_xm());

        m_editor_scene->frustum_culling(frustum);

        // Pick entity
        if (!ImGuizmo::IsUsing() && DX_INPUT::is_mouse_button_pressed(m_d3d_app->get_glfw_window(), mouse::ButtonLeft))
        {
            auto [mouse_pos_x, mouse_pos_y] = ImGui::GetMousePos();
            mouse_pos_x -= m_viewer_spec.lower_bound.x;
            mouse_pos_y -= m_viewer_spec.lower_bound.y;
            auto pick_anything = m_editor_scene->pick_entity(m_selected_entity, *m_camera, mouse_pos_x, mouse_pos_y);
            if (m_viewer_focused && m_viewer_hovered && pick_anything)
            {
                m_scene_hierarchy_panel.set_selected_entity(m_selected_entity);
            }
        }
    }

    void PBRViewer::render_shadow()
    {
        using namespace DirectX;
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();
        auto&& shadow_effect = ShadowEffect::get();
        auto&& cascade_shadow_manager = CascadedShadowManager::get();
        auto viewport = cascade_shadow_manager.get_shadow_viewport();

        shadow_effect.set_view_matrix(m_light_camera->get_view_xm());
        m_editor_scene->update_cascaded_shadow([this, &cascade_shadow_manager] (const DirectX::BoundingBox &bounding_box){
            cascade_shadow_manager.update_frame(*m_camera, *m_light_camera, bounding_box);
        });

        shadow_effect.set_default_render();
        d3d_device_context->RSSetViewports(1, &viewport);
        for (size_t cascade_index = 0; cascade_index < cascade_shadow_manager.cascade_levels; ++cascade_index)
        {
            ID3D11RenderTargetView* null_rtv = nullptr;
            ID3D11DepthStencilView* dsv = cascade_shadow_manager.get_cascade_depth_stencil_view(cascade_index);
            d3d_device_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
            d3d_device_context->OMSetRenderTargets(1, &null_rtv, dsv);

            XMMATRIX shadow_proj = cascade_shadow_manager.get_shadow_project_xm(cascade_index);
            shadow_effect.set_proj_matrix(shadow_proj);

            m_editor_scene->render_static_mesh(d3d_device_context, shadow_effect, true);
        }
    }

    void PBRViewer::render_gbuffer()
    {
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();

        // Only need to clear depth buffer, since we use skybox sampling to replace no-written pixel(far plane)
        // Use depth buffer to reconstruct position, only position in frustum can be draw
        // Reverse z-buffer, far plane is 0.0
        for (auto rtv : m_gbuffer_rtvs)
        {
            d3d_device_context->ClearRenderTargetView(rtv, s_color);
        }

        d3d_device_context->ClearDepthStencilView(m_depth_buffer->get_depth_stencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        D3D11_VIEWPORT viewport = m_camera->get_viewport();
        d3d_device_context->RSSetViewports(1, &viewport);

        DeferredPBREffect::get().set_gbuffer_render();
        d3d_device_context->OMSetRenderTargets(static_cast<uint32_t>(m_gbuffers.size()), m_gbuffer_rtvs.data(), m_depth_buffer->get_depth_stencil());
        m_editor_scene->render_static_mesh(d3d_device_context, DeferredPBREffect::get());
        d3d_device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void PBRViewer::taa_pass()
    {
        static bool first_frame = true;
        static uint32_t taa_frame_counter = 0;
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();
        D3D11_VIEWPORT viewport = m_camera->get_viewport();

        set_shadow_paras();
        if (first_frame)
        {
            DeferredPBREffect::get().deferred_lighting_pass(d3d_device_context, m_cur_buffer->get_render_target(),
                                                            m_gbuffer_srvs.data(), m_camera->get_viewport());
            TAAEffect::get().render(d3d_device_context, m_cur_buffer->get_shader_resource(), m_cur_buffer->get_shader_resource(),
                                    m_gbuffers[3]->get_shader_resource(), m_depth_buffer->get_shader_resource(),
                                    m_viewer_buffer->get_render_target(), viewport);
            first_frame = false;
        } else
        {
            DeferredPBREffect::get().deferred_lighting_pass(d3d_device_context, m_cur_buffer->get_render_target(),
                                                            m_gbuffer_srvs.data(), m_camera->get_viewport());
            TAAEffect::get().render(d3d_device_context, m_history_buffer->get_shader_resource(), m_cur_buffer->get_shader_resource(),
                                    m_gbuffers[3]->get_shader_resource(), m_depth_buffer->get_shader_resource(),
                                    m_viewer_buffer->get_render_target(), viewport);
        }

        d3d_device_context->CopyResource(m_history_buffer->get_texture(), m_cur_buffer->get_texture());

        taa_frame_counter = (taa_frame_counter + 1) % taa::s_taa_sample;
    }

    void PBRViewer::render_skybox()
    {
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();
        D3D11_VIEWPORT skybox_viewport = m_camera->get_viewport();
        skybox_viewport.MinDepth = 1.0f;
        skybox_viewport.MaxDepth = 1.0f;
        auto render_target_view = m_viewer_buffer->get_render_target();

        d3d_device_context->RSSetViewports(1, &skybox_viewport);

        SimpleSkyboxEffect::get().set_skybox_render();
        SimpleSkyboxEffect::get().set_view_matrix(m_camera->get_view_xm());
        SimpleSkyboxEffect::get().set_proj_matrix(m_camera->get_proj_xm());
        SimpleSkyboxEffect::get().apply(d3d_device_context);

        d3d_device_context->OMSetRenderTargets(1, &render_target_view, m_depth_buffer->get_depth_stencil());
        m_editor_scene->render_skybox(d3d_device_context, SimpleSkyboxEffect::get());
        d3d_device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void PBRViewer::set_shadow_paras()
    {
        using namespace DirectX;

        static XMMATRIX s_transform = {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f
        };

        auto&& deferred_pbr_effect = DeferredPBREffect::get();
        auto&& cascade_shadow_manager = CascadedShadowManager::get();

        std::array<XMFLOAT4, 8> scales = {};
        std::array<XMFLOAT4, 8> offsets = {};
        // From NDC [-1, 1]^2 to texture coordinates [0, 1]^2
        for (size_t cascade_index = 0; cascade_index < cascade_shadow_manager.cascade_levels; ++cascade_index)
        {
            XMMATRIX shadow_texture = cascade_shadow_manager.get_shadow_project_xm(cascade_index) * s_transform;
            scales[cascade_index].x  = XMVectorGetX(shadow_texture.r[0]);
            scales[cascade_index].y = XMVectorGetY(shadow_texture.r[1]);
            scales[cascade_index].z = XMVectorGetZ(shadow_texture.r[2]);
            scales[cascade_index].w = 1.0f;
            XMStoreFloat3((XMFLOAT3 *)(offsets.data() + cascade_index), shadow_texture.r[3]);
        }

        deferred_pbr_effect.set_cascade_frustums_eye_space_depths(cascade_shadow_manager.get_cascade_partitions());
        deferred_pbr_effect.set_cascade_offsets(offsets.data());
        deferred_pbr_effect.set_cascade_scales(scales.data());
        deferred_pbr_effect.set_shadow_view_matrix(m_light_camera->get_view_xm());
        deferred_pbr_effect.set_shadow_texture_array(cascade_shadow_manager.get_cascades_output());
        deferred_pbr_effect.set_light_direction(m_light_camera->get_look_axis());
    }
}