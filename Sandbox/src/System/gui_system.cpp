//
// Created by ZZK on 2024/3/13.
//

#include <Sandbox/System/gui_system.h>
#include <IconsFontAwesome6.h>
#include <Toy/Runtime/events.h>

namespace toy::editor
{
    GuiSystem::GuiSystem(GLFWwindow *glfw_window, ID3D11Device *device, ID3D11DeviceContext *device_context)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;                   // Enable keyboard controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;                    // Enable gamepad controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;                       // Enable docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;                     // Enable multi-viewport / platform windows

        ImGui::StyleColorsDark();                                               // Set ImGui style

        ImGui_ImplGlfw_InitForOther(glfw_window, true);    // Set window platform
        ImGui_ImplDX11_Init(device, device_context);                            // Set render backend

        // Load fonts
        float base_font_size = 18.0f;
        float bold_font_size = 20.0f;
        float icon_font_size = base_font_size * 2.0f / 3.0f;                    // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

        // Merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        ImFontConfig icons_config{};
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = icon_font_size;
        auto bold_font = io.Fonts->AddFontFromFileTTF("../data/fonts/AdobeFonts/SourceCodePro-Bold.ttf", bold_font_size,
                                                        nullptr, io.Fonts->GetGlyphRangesDefault());
        auto default_font = io.Fonts->AddFontFromFileTTF("../data/fonts/AdobeFonts/SourceCodePro-Regular.ttf", base_font_size,
                                                            nullptr, io.Fonts->GetGlyphRangesDefault());
        io.FontDefault = default_font;
        io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_FAS, icon_font_size, &icons_config, icons_ranges);
        io.Fonts->Build();

        // Set dark color theme by default
        set_dark_theme();

        // Register frame-begin and frame-end functions
        runtime::on_frame_begin = [this] (float delta_time) {
            this->frame_begin();
        };
        runtime::on_frame_end = [this] (float delta_time) {
            this->frame_end();
        };
    }

    void GuiSystem::frame_begin()
    {
        // ImGui frame
        ImGui_ImplDX11_NewFrame();
        //ImGui_ImplWin32_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuizmo::BeginFrame();

        // Begin docking space
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        {
            window_flags |= ImGuiWindowFlags_NoBackground;
        }

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowMinSize.x = 360.0f;
    }

    void GuiSystem::frame_end()
    {
        // End docking space
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and render additional platform windows
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void GuiSystem::release()
    {
        if (!has_released)
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        has_released = true;
    }

    void GuiSystem::set_dark_theme()
    {
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Menubar
        colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    void GuiSystem::set_gizmo_style()
    {
        auto&& gizmo_style = ImGuizmo::GetStyle();

        gizmo_style.TranslationLineThickness   = 4.0f;
        gizmo_style.TranslationLineArrowSize   = 7.0f;
        gizmo_style.RotationLineThickness      = 3.0f;
        gizmo_style.RotationOuterLineThickness = 4.0f;
        gizmo_style.ScaleLineThickness         = 4.0f;
        gizmo_style.ScaleLineCircleSize        = 7.0f;
        gizmo_style.HatchedAxisLineThickness   = 7.0f;
        gizmo_style.CenterCircleSize           = 7.0f;
    }
}