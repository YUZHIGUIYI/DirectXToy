//
// Created by ZHIKANG on 2023/6/24.
//

#include <Toy/ImGui/imgui_viewport.h>

namespace toy
{
    void Viewport::set_viewport(std::string_view viewport_name, ID3D11ShaderResourceView* srv, int32_t &width, int32_t &height)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

        ImGui::Begin(viewport_name.data());

        auto viewport_size = ImGui::GetContentRegionAvail();
        width = static_cast<int32_t>(viewport_size.x);
        height = static_cast<int32_t>(viewport_size.y);

        ImGui::Image(srv, ImVec2{ viewport_size.x, viewport_size.y });

        ImGui::End();
        ImGui::PopStyleVar();
    }
}