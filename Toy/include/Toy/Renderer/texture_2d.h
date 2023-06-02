//
// Created by ZZK on 2023/6/1.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{

    class Texture2DBase
    {
    public:
        Texture2DBase(ID3D11Device* device, const CD3D11_TEXTURE2D_DESC& tex_desc, const CD3D11_SHADER_RESOURCE_VIEW_DESC& srv_desc);
        virtual ~Texture2DBase() = default;

        ID3D11Texture2D* get_texture() { return m_texture.Get(); }
        ID3D11ShaderResourceView* get_shader_resource() { return m_texture_srv.Get(); }

        [[nodiscard]] uint32_t get_width() const { return m_width; }
        [[nodiscard]] uint32_t get_height() const { return m_height; }

        // Set debug object name
        virtual void set_debug_object_name(std::string_view name);

    protected:
        com_ptr<ID3D11Texture2D> m_texture;
        com_ptr<ID3D11ShaderResourceView> m_texture_srv;
        uint32_t m_width{};
        uint32_t m_height{};
    };

    enum class DepthStencilBitsFlag
    {
        Depth_16Bits = 0,
        Depth_24Bits_Stencil_8Bits = 1,
        Depth_32Bits = 2,
        Depth_32Bits_Stencil_8Bits_Unused_24Bits = 3,
    };

    class Depth2D : public Texture2DBase
    {
    public:
        Depth2D(ID3D11Device* device, uint32_t width, uint32_t height,
                DepthStencilBitsFlag depth_stencil_bits_flag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                uint32_t bind_flags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
        ~Depth2D() override = default;

        ID3D11DepthStencilView* get_depth_stencil() { return m_texture_dsv.Get(); }

        void set_debug_object_name(std::string_view name) override;

    private:
        com_ptr<ID3D11DepthStencilView> m_texture_dsv;
    };
}




