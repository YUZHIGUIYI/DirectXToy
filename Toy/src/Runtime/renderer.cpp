//
// Created by ZZK on 2024/3/13.
//

#include <Toy/Runtime/renderer.h>

namespace toy::runtime
{
    Renderer::Renderer()
    {
        init_backend();
    }

    Renderer::~Renderer()
    {
        if (!m_has_released)
        {
            release();
        }
    }

    void Renderer::reset_render_target()
    {
        ID3D11RenderTargetView* rtv =  get_back_buffer_rtv();
        m_d3d_immediate_context->OMSetRenderTargets(1, &rtv, nullptr);
        ++m_frame_count;
    }

    void Renderer::present()
    {
        m_swap_chain->Present(0, m_is_dxgi_flip_model ? DXGI_PRESENT_ALLOW_TEARING : 0);
    }

    void Renderer::release()
    {
        if (m_d3d_immediate_context)
        {
            m_d3d_immediate_context->ClearState();
        }
    }

    void Renderer::init_backend()
    {

    }

    void Renderer::init_effects()
    {

    }

    void Renderer::on_resize()
    {

    }
}