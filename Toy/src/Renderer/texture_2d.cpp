//
// Created by ZZK on 2023/6/1.
//

#include <Toy/Renderer/texture_2d.h>

namespace toy
{
    static DXGI_FORMAT get_depth_texture_format(DepthStencilBitsFlag flag)
    {
        switch (flag)
        {
            case DepthStencilBitsFlag::Depth_16Bits:                                return DXGI_FORMAT_R16_TYPELESS;
            case DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits:                  return DXGI_FORMAT_R24G8_TYPELESS;
            case DepthStencilBitsFlag::Depth_32Bits:                                return DXGI_FORMAT_R32_TYPELESS;
            case DepthStencilBitsFlag::Depth_32Bits_Stencil_8Bits_Unused_24Bits:    return DXGI_FORMAT_R32G8X24_TYPELESS;
            default:                                                                return DXGI_FORMAT_UNKNOWN;
        }
    }

    static DXGI_FORMAT get_depth_srv_format(DepthStencilBitsFlag flag)
    {
        switch (flag)
        {
            case DepthStencilBitsFlag::Depth_16Bits:                                return DXGI_FORMAT_R16_FLOAT;
            case DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits:                  return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case DepthStencilBitsFlag::Depth_32Bits:                                return DXGI_FORMAT_R32_FLOAT;
            case DepthStencilBitsFlag::Depth_32Bits_Stencil_8Bits_Unused_24Bits:    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            default:                                                                return DXGI_FORMAT_UNKNOWN;
        }
    }

    static DXGI_FORMAT get_depth_dsv_format(DepthStencilBitsFlag flag)
    {
        switch (flag)
        {
            case DepthStencilBitsFlag::Depth_16Bits:                                return DXGI_FORMAT_D16_UNORM;
            case DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits:                  return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case DepthStencilBitsFlag::Depth_32Bits:                                return DXGI_FORMAT_D32_FLOAT;
            case DepthStencilBitsFlag::Depth_32Bits_Stencil_8Bits_Unused_24Bits:    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            default:                                                                return DXGI_FORMAT_UNKNOWN;
        }
    }

    Texture2DBase::Texture2DBase(ID3D11Device *device, const CD3D11_TEXTURE2D_DESC &tex_desc,
                                    const CD3D11_SHADER_RESOURCE_VIEW_DESC &srv_desc)
    : m_width(tex_desc.Width), m_height(tex_desc.Height)
    {
        m_texture.Reset();
        m_texture_srv.Reset();

        device->CreateTexture2D(&tex_desc, nullptr, m_texture.GetAddressOf());
        if (!m_texture)
        {
            DX_CORE_CRITICAL("Fail to create 2d texture");
        }

        // Update with actual generated mip levels and other data
        D3D11_TEXTURE2D_DESC desc{};
        m_texture->GetDesc(&desc);

        // Create shader resource view
        if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
        {
            device->CreateShaderResourceView(m_texture.Get(), &srv_desc, m_texture_srv.GetAddressOf());
        }
    }

    void Texture2DBase::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    Depth2D::Depth2D(ID3D11Device *device, uint32_t width, uint32_t height,
                        toy::DepthStencilBitsFlag depth_stencil_bits_flag, uint32_t bind_flags)
    : Texture2DBase(device,
                    CD3D11_TEXTURE2D_DESC(get_depth_texture_format(depth_stencil_bits_flag), width, height, 1, 1, bind_flags),
                    CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D, get_depth_srv_format(depth_stencil_bits_flag)))
    {
        if (bind_flags & D3D11_BIND_DEPTH_STENCIL)
        {
            CD3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc(D3D11_DSV_DIMENSION_TEXTURE2D, get_depth_dsv_format(depth_stencil_bits_flag));
            device->CreateDepthStencilView(m_texture.Get(), &dsv_desc, m_texture_dsv.GetAddressOf());
        }
    }

    void Depth2D::set_debug_object_name(std::string_view name)
    {
        // TODO
    }
}





