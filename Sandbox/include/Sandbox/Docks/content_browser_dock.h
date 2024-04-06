//
// Created by ZZK on 2024/3/17.
//

#pragma once

#include <Sandbox/Docks/dock.h>

namespace toy::editor
{
    struct ContentBrowserDock final : Dock
    {
    public:
        explicit ContentBrowserDock(std::string &&dock_name);

        ~ContentBrowserDock() override = default;

        void on_render(float delta_time) override;

    private:
        void set_browser_cache_path(const std::filesystem::path &path);

    private:
        std::string m_dock_name;
        std::filesystem::path m_browser_cache_path;
        float m_icon_scale = 0.78f;
    };
}
