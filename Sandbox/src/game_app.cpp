//
// Created by ZZK on 2023/5/17.
//

#include <Sandbox/game_app.h>

namespace toy
{
    game_app_c::game_app_c(HINSTANCE hinstance, const std::string &window_name, int32_t init_width, int32_t init_height)
    : d3d_application_c(hinstance, window_name, init_width, init_height)
    {

    }

    game_app_c::~game_app_c()
    {

    }

    void game_app_c::init()
    {
        d3d_application_c::init();
    }

    void game_app_c::on_resize()
    {
        d3d_application_c::on_resize();
    }

    void game_app_c::update_scene(float dt)
    {

    }

    void game_app_c::draw_scene()
    {
        assert(class_d3d_immediate_context_);
        assert(class_swap_chain_);

        static float color[4] { 1.0f, 0.0f, 0.0f, 1.0f };
        class_d3d_immediate_context_->ClearRenderTargetView(class_render_target_view_.Get(), color);
        class_d3d_immediate_context_->ClearDepthStencilView(class_depth_stencil_view_.Get(),
                                                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}






































