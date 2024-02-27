//
// Created by ZZK on 2024/1/22.
//

#include <Sandbox/console_dock.h>

namespace toy
{
    ConsoleDock::ConsoleDock()
    : m_console_sink(std::make_shared<logger::RingbufferConsoleSinkMt>(80))
    {
        Logger::get().register_console_sink(m_console_sink);
    }

    void ConsoleDock::on_console_render()
    {
        if (!m_console_sink) return;

        ImGui::Begin("Console");

        static ImGuiTextFilter filter = {};
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
        filter.Draw("FILTER", 180);
        ImGui::PopStyleVar();
        ImGui::SameLine();
        if (ImGui::SmallButton("CLEAR"))
        {
            m_console_sink->clear_log();
        }
        ImGui::Separator();

        // Display every line as a separate entry so we can change their color or add custom widgets
        // If only want raw text, use ImGui::TextUnformatted(log.begin(), log.end())
        // If have thousands of entries, this approach may be inefficient, you can seek and display only the lines that are visible
        // - CalcListClipping() is a helper to compute this information
        auto height = -ImGui::GetStyle().ItemSpacing.y - ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2{ 0.0f, height }, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear"))
            {
                m_console_sink->clear_log();
            }
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 4.0f, 1.0f });
        const auto& log_items = m_console_sink->get_cache_log();
        for (auto&& message_pair : log_items)
        {
            if (!filter.PassFilter(message_pair.second.c_str()))
            {
                continue;
            }

            const auto& colorization = m_console_sink->get_level_colorization(message_pair.first);
            ImVec4 color = { colorization[0], colorization[1], colorization[2], colorization[3] };
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextWrapped("%s", message_pair.second.c_str());
            ImGui::PopStyleColor();
        }

        if (m_console_sink->has_new_entries())
        {
            ImGui::SetScrollHereY();
        }
        m_console_sink->set_new_entries(false);

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // TODO: Support command line
        static std::array<char, 64> input_buffer;
        input_buffer.fill(0);

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        if (ImGui::InputText("ENTER COMMAND", input_buffer.data(), input_buffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            // Copy from c-string, in order to remove trailing zeros
            // std::string command = input_buffer.data();
            // Demonstrate keeping automatic focus on the input box
            if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                    !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
            {
                ImGui::SetKeyboardFocusHere(-1);  // Automatic focus previous widget
            }
        }
        ImGui::PopItemWidth();

        ImGui::End();
    }
}
















