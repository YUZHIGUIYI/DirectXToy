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

    template<typename T>
    static constexpr T compute_mipmap_levels(T width, T height)
    {
        T levels = 1;
        while ((width|height) >> levels)
        {
            ++levels;
        }
        return levels;
    }

    // Texture 2d base
    Texture2DBase::Texture2DBase(ID3D11Device *device, const CD3D11_TEXTURE2D_DESC &tex_desc,
                                    const CD3D11_SHADER_RESOURCE_VIEW_DESC &srv_desc)
    : m_width(tex_desc.Width), m_height(tex_desc.Height)
    {
        m_texture.Reset();
        m_texture_srv.Reset();

        auto modified_texture_desc = tex_desc;
        // Only available for TextureCube and Texture2D
        // FIXME
        if (tex_desc.MipLevels == 0)
        {
            modified_texture_desc.MipLevels = compute_mipmap_levels(tex_desc.Width, tex_desc.Height);
            modified_texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            modified_texture_desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }

        device->CreateTexture2D(&modified_texture_desc, nullptr, m_texture.GetAddressOf());
        if (!m_texture)
        {
            DX_CORE_CRITICAL("Fail to create 2d texture");
        }

        // Update with actual generated mip levels and other data
        D3D11_TEXTURE2D_DESC desc{};
        m_texture->GetDesc(&desc);

        // Create shader resource view
        if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        {
            device->CreateShaderResourceView(m_texture.Get(), &srv_desc, m_texture_srv.GetAddressOf());
        }
    }

    void Texture2DBase::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    // Texture 2d
    Texture2D::Texture2D(ID3D11Device *device, uint32_t width, uint32_t height, DXGI_FORMAT format, uint32_t mip_levels, uint32_t bind_flags)
    : Texture2DBase(device, CD3D11_TEXTURE2D_DESC{ format, width, height, 1, mip_levels, bind_flags },
                    CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_TEXTURE2D, format })
    {
        D3D11_TEXTURE2D_DESC desc{};
        m_texture->GetDesc(&desc);
        m_mip_levels = desc.MipLevels;
        if (bind_flags & D3D11_BIND_RENDER_TARGET)
        {
            device->CreateRenderTargetView(m_texture.Get(), nullptr, m_texture_rtv.GetAddressOf());
        }
//        if (bind_flags & D3D11_BIND_UNORDERED_ACCESS)
//        {
//            D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
//            uav_desc.Format = desc.Format;
//            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
//            uav_desc.Texture2D.MipSlice = 0;
//            device->CreateUnorderedAccessView(m_texture.Get(), &uav_desc, m_texture_uav.GetAddressOf());
//        }
    }

    void Texture2D::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    // Texture2DMS
    Texture2DMS::Texture2DMS(ID3D11Device *device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                                const DXGI_SAMPLE_DESC &sample_desc, uint32_t bind_flags)
    : Texture2DBase(device, CD3D11_TEXTURE2D_DESC{ format, width, height, 1, 1, bind_flags,
                    D3D11_USAGE_DEFAULT, 0, sample_desc.Count, sample_desc.Quality },
                    CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_TEXTURE2DMS, format }),
    m_msaa_samples(sample_desc.Count)
    {
        if (bind_flags & D3D11_BIND_RENDER_TARGET)
        {
            device->CreateRenderTargetView(m_texture.Get(), nullptr, m_texture_rtv.GetAddressOf());
        }
    }

    void Texture2DMS::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    // Texture cube
    TextureCube::TextureCube(ID3D11Device *device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                                uint32_t mip_levels, uint32_t bind_flags)
    : Texture2DBase(device, CD3D11_TEXTURE2D_DESC{ format, width, height, 6, mip_levels, bind_flags, D3D11_USAGE_DEFAULT,
                    0, 1, 0, D3D11_RESOURCE_MISC_TEXTURECUBE },
                    CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_TEXTURECUBE, format })
    {
        D3D11_TEXTURE2D_DESC desc{};
        m_texture->GetDesc(&desc);
        m_mip_levels = desc.MipLevels;
        if (bind_flags & D3D11_BIND_RENDER_TARGET)
        {
            // Single sub resource
            m_render_target_elements.reserve(6);
            for (uint32_t i = 0; i < 6; ++i)
            {
                CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{
                    D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
                    format,
                    0, i, 1
                };
                com_ptr<ID3D11RenderTargetView> rtv = nullptr;
                device->CreateRenderTargetView(m_texture.Get(), &rtv_desc, rtv.GetAddressOf());
                m_render_target_elements.push_back(rtv);
            }

            // Complete resources
            CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{D3D11_RTV_DIMENSION_TEXTURE2DARRAY, format, 0};
            device->CreateRenderTargetView(m_texture.Get(), &rtv_desc, m_texture_array_rtv.GetAddressOf());
        }

//        if (bind_flags & D3D11_BIND_UNORDERED_ACCESS)
//        {
//            // Single sub resource
//            m_unordered_access_elements.reserve(6);
//            for (uint32_t i = 0; i <  6; ++i)
//            {
//                CD3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{
//                    D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
//                    format,
//                    0, i, 1
//                };
//
//                com_ptr<ID3D11UnorderedAccessView> uav = nullptr;
//                device->CreateUnorderedAccessView(m_texture.Get(), &uav_desc, uav.GetAddressOf());
//                m_unordered_access_elements.push_back(uav);
//            }
//        }

//        if (bind_flags & D3D11_BIND_SHADER_RESOURCE)
//        {
//            // Single sub resource
//            m_shader_resource_elements.reserve(6);
//            for (uint32_t i = 0; i < 6; ++i)
//            {
//                CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
//                    D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
//                    format,
//                    0, uint32_t(-1),
//                    i, 1
//                };
//
//                com_ptr<ID3D11ShaderResourceView> srv = nullptr;
//                device->CreateShaderResourceView(m_texture.Get(), &srv_desc, srv.GetAddressOf());
//                m_shader_resource_elements.push_back(srv);
//            }
//        }
    }

    void TextureCube::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    // Texture 2d array
    Texture2DArray::Texture2DArray(ID3D11Device *device, uint32_t width, uint32_t height, DXGI_FORMAT format, uint32_t array_size, uint32_t mip_levels, uint32_t bind_flags)
    : Texture2DBase(device, CD3D11_TEXTURE2D_DESC{ format, width, height, array_size, mip_levels, bind_flags },
                    CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_TEXTURE2DARRAY, format }),
    m_array_size(array_size)
    {
        D3D11_TEXTURE2D_DESC desc{};
        m_texture->GetDesc(&desc);
        m_mip_levels = desc.MipLevels;
        // Render target view
        if (bind_flags & D3D11_BIND_RENDER_TARGET)
        {
            m_render_target_elements.reserve(array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{
                    D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
                    format,
                    0, i, 1
                };

                com_ptr<ID3D11RenderTargetView> rtv = nullptr;
                device->CreateRenderTargetView(m_texture.Get(), &rtv_desc, rtv.GetAddressOf());
                m_render_target_elements.push_back(rtv);
            }
        }
        // Unordered access view
        if (bind_flags & D3D11_BIND_UNORDERED_ACCESS)
        {
            m_unordered_access_elements.reserve(array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{
                    D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
                    format,
                    0, i, 1
                };

                com_ptr<ID3D11UnorderedAccessView> uav = nullptr;
                device->CreateUnorderedAccessView(m_texture.Get(), &uav_desc, uav.GetAddressOf());
                m_unordered_access_elements.push_back(uav);
            }
        }
        // Shader resource view
        if (bind_flags & D3D11_BIND_SHADER_RESOURCE)
        {
            m_shader_resource_elements.reserve(array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
                    D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
                    format,
                    0, uint32_t(-1),
                    i, 1
                };

                com_ptr<ID3D11ShaderResourceView> srv = nullptr;
                device->CreateShaderResourceView(m_texture.Get(), &srv_desc, srv.GetAddressOf());
                m_shader_resource_elements.push_back(srv);
            }
        }
    }

    void Texture2DArray::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    // Texture 2D MS array
    Texture2DMSArray::Texture2DMSArray(ID3D11Device *device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                                        uint32_t array_size, const DXGI_SAMPLE_DESC &sample_desc, uint32_t bind_flags)
    : Texture2DBase(device, CD3D11_TEXTURE2D_DESC{ format, width, height, array_size, 1, bind_flags,
                D3D11_USAGE_DEFAULT, 0, sample_desc.Count, sample_desc.Quality },
                    CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY, format }),
    m_array_size(array_size), m_msaa_samples(sample_desc.Count)
    {
        if (bind_flags & D3D11_BIND_RENDER_TARGET)
        {
            // SubResources of render target views
            m_render_target_elements.reserve(m_array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{
                    D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY,
                    format,
                    0,
                    i, 1
                };

                com_ptr<ID3D11RenderTargetView> rtv = nullptr;
                device->CreateRenderTargetView(m_texture.Get(), &rtv_desc, rtv.GetAddressOf());
                m_render_target_elements.push_back(rtv);
            }

            // Complete resource
            CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{ D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY, format, 0 };
            device->CreateRenderTargetView(m_texture.Get(), &rtv_desc, m_texture_array_rtv.GetAddressOf());
        }

        if (bind_flags & D3D11_BIND_SHADER_RESOURCE)
        {
            // SubResources of shader resource views
            m_shader_resource_elements.reserve(array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_SHADER_RESOURCE_VIEW_DESC srv_element_desc{
                    D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY,
                    format,
                    0, uint32_t(-1),
                    i, 1
                };

                com_ptr<ID3D11ShaderResourceView> srv = nullptr;
                device->CreateShaderResourceView(m_texture.Get(), &srv_element_desc, srv.GetAddressOf());
                m_shader_resource_elements.push_back(srv);
            }
        }
    }

    void Texture2DMSArray::set_debug_object_name(std::string_view name)
    {
#if (defined(_DEBUG) || GRAPHICS_DEBUGGER_OBJECT_NAME)
        // TODO
#endif
    }

    // Depth 2D
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

    // Depth 2d msaa
    Depth2DMS::Depth2DMS(ID3D11Device* device, uint32_t width, uint32_t height,
                            const DXGI_SAMPLE_DESC& sampleDesc, DepthStencilBitsFlag depthStencilBitsFlag, uint32_t bindFlags)
    : Texture2DBase(device,
                CD3D11_TEXTURE2D_DESC(get_depth_texture_format(depthStencilBitsFlag), width, height, 1, 1, bindFlags,
                                        D3D11_USAGE_DEFAULT, 0, sampleDesc.Count, sampleDesc.Quality),
                CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2DMS, get_depth_srv_format(depthStencilBitsFlag))),
    m_msaa_samples(sampleDesc.Count)
    {
        if (bindFlags & D3D11_BIND_DEPTH_STENCIL)
        {
            CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS, get_depth_dsv_format(depthStencilBitsFlag));
            device->CreateDepthStencilView(m_texture.Get(), &dsvDesc, m_texture_dsv.GetAddressOf());
        }
    }

    void Depth2DMS::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    // Depth 2d array
    Depth2DArray::Depth2DArray(ID3D11Device *device, uint32_t width, uint32_t height, uint32_t array_size,
                                toy::DepthStencilBitsFlag depth_stencil_bits_flag, uint32_t bind_flags)
    : Texture2DBase(device, CD3D11_TEXTURE2D_DESC{get_depth_texture_format(depth_stencil_bits_flag), width, height, array_size,
                        1, bind_flags },
                    CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_TEXTURE2DARRAY, get_depth_srv_format(depth_stencil_bits_flag) }),
    m_array_size(array_size)
    {
        if (bind_flags & D3D11_BIND_DEPTH_STENCIL)
        {
            // SubResources of depth stencil views
            m_depth_stencil_elements.reserve(m_array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_DEPTH_STENCIL_VIEW_DESC dsv_element_desc{
                    D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
                    get_depth_dsv_format(depth_stencil_bits_flag),
                    0, i, 1
                };

                com_ptr<ID3D11DepthStencilView> dsv = nullptr;
                device->CreateDepthStencilView(m_texture.Get(), &dsv_element_desc, dsv.GetAddressOf());
                m_depth_stencil_elements.push_back(dsv);
            }

            // Complete resource
            CD3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{ D3D11_DSV_DIMENSION_TEXTURE2DARRAY, get_depth_dsv_format(depth_stencil_bits_flag), 0 };
            device->CreateDepthStencilView(m_texture.Get(), &dsv_desc, m_depth_array_dsv.GetAddressOf());
        }

        if (bind_flags & D3D11_BIND_SHADER_RESOURCE)
        {
            // SubResources of shader resource views
            m_shader_resource_elements.reserve(array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_SHADER_RESOURCE_VIEW_DESC srv_element_desc{
                    D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
                    get_depth_srv_format(depth_stencil_bits_flag),
                    0, 1,
                    i, 1
                };

                com_ptr<ID3D11ShaderResourceView> srv = nullptr;
                device->CreateShaderResourceView(m_texture.Get(), &srv_element_desc, srv.GetAddressOf());
                m_shader_resource_elements.push_back(srv);
            }
        }
    }

    void Depth2DArray::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    // Depth2DMSArray
    Depth2DMSArray::Depth2DMSArray(ID3D11Device *device, uint32_t width, uint32_t height, uint32_t array_size,
                                    const DXGI_SAMPLE_DESC &sample_desc,
                                    toy::DepthStencilBitsFlag depth_stencil_bits_flag, uint32_t bind_flags)
    : Texture2DBase(device, CD3D11_TEXTURE2D_DESC{ get_depth_texture_format(depth_stencil_bits_flag), width, height, array_size,
                        1, bind_flags, D3D11_USAGE_DEFAULT, 0, sample_desc.Count, sample_desc.Quality },
                    CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY, get_depth_srv_format(depth_stencil_bits_flag) }
                    ),
    m_array_size(array_size), m_msaa_samples(sample_desc.Count)
    {
        if (bind_flags & D3D11_BIND_DEPTH_STENCIL)
        {
            // SubResources of depth stencil views
            m_depth_stencil_elements.reserve(array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_DEPTH_STENCIL_VIEW_DESC dsv_element_desc{
                    D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY,
                    get_depth_dsv_format(depth_stencil_bits_flag),
                    0, i, 1
                };

                com_ptr<ID3D11DepthStencilView> dsv = nullptr;
                device->CreateDepthStencilView(m_texture.Get(), &dsv_element_desc, dsv.GetAddressOf());
                m_depth_stencil_elements.push_back(dsv);
            }

            // Complete resource
            CD3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{
                D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY,
                get_depth_dsv_format(depth_stencil_bits_flag), 0 };
            device->CreateDepthStencilView(m_texture.Get(), &dsv_desc, m_depth_array_dsv.GetAddressOf());
        }

        if (bind_flags & D3D11_BIND_SHADER_RESOURCE)
        {
            // SubResources of shader resource views
            m_shader_resource_elements.reserve(array_size);
            for (uint32_t i = 0; i < array_size; ++i)
            {
                CD3D11_SHADER_RESOURCE_VIEW_DESC srv_element_desc{
                    D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY,
                    get_depth_srv_format(depth_stencil_bits_flag),
                    0, 1,
                    i, 1
                };

                com_ptr<ID3D11ShaderResourceView> srv = nullptr;
                device->CreateShaderResourceView(m_texture.Get(), &srv_element_desc, srv.GetAddressOf());
                m_shader_resource_elements.push_back(srv);
            }
        }
    }

    void Depth2DMSArray::set_debug_object_name(std::string_view name)
    {
        // TODO
    }
}





