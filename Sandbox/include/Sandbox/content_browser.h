//
// Created by ZZK on 2023/12/10.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    struct ContentBrowser
    {
    public:
        explicit ContentBrowser();

        void on_browser_render();

    private:
        void set_browser_cache_path(const std::filesystem::path &path);

    private:
        std::filesystem::path browser_cache_path;
        float icon_scale = 0.78f;
    };
}
