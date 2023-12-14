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
        void set_cache_path(const std::filesystem::path &path);

    private:
        std::filesystem::directory_iterator cache_iter;
        std::filesystem::path cache_path;
        std::filesystem::path root_path;
        float icon_scale = 0.75f;
    };
}
