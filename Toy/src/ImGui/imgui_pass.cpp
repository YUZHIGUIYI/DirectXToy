//
// Created by ZHIKANG on 2023/6/4.
//

#include <Toy/ImGui/imgui_pass.h>
#include <IconsFontAwesome6.h>

namespace toy
{
    void ImGuiPass::init(GLFWwindow *glfw_window, ID3D11Device *device, ID3D11DeviceContext *device_context)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;                   // Enable keyboard controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;                    // Enable gamepad controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;                       // Enable docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;                   // Enable multi-viewport / platform windows

        ImGui::StyleColorsDark();                                               // Set ImGui style

        //ImGui_ImplWin32_Init(m_main_wnd);                                     // Set window platform
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
    }

    void ImGuiPass::begin()
    {
        // ImGui frame
        ImGui_ImplDX11_NewFrame();
        //ImGui_ImplWin32_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiPass::end()
    {
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

    void ImGuiPass::release()
    {
        ImGui_ImplDX11_Shutdown();
        //ImGui_ImplWin32_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiPass::set_dark_theme()
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
    }
}