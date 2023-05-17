//
// Created by ZZK on 2023/5/17.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    class game_app_c : public d3d_application_c
    {
    public:
        game_app_c(HINSTANCE hinstance, const std::string& window_name, int32_t init_width, int32_t init_height);
        ~game_app_c() override;

        void init() override;
        void on_resize() override;
        void update_scene(float dt) override;
        void draw_scene() override;
    };
}



































